#include "redundant_history_encoder.h"
#include "utils.h"

#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

int _RedundantHistoryEncoder::get_redundancy()
{
	return _redundancy;
}

void _RedundantHistoryEncoder::set_redundancy(int p_redundancy)
{
	if(p_redundancy <= 0)
	{
		_logger->warning(vformat("Attempting to set redundancy to %d, which would send no data!", p_redundancy));
		return ;
	}

	_redundancy = p_redundancy;
}

void _RedundantHistoryEncoder::set_properties(TypedArray<PropertyEntry> properties)
{
	if(_properties != properties)
	{
		_version = (_version + 1) % 256;
		_properties = properties.duplicate();
	}
}

Array _RedundantHistoryEncoder::encode(int tick, TypedArray<PropertyEntry> properties)
{
	if(_history->is_empty())
	{
		return Array();
	}
	Array data = Array();

	for(int i=0; i<Math::min(_redundancy, _history->size()); i+=1)
	{
		int offset_tick = tick - i;
		if(offset_tick < _history->get_earliest_tick())
			break;

		Ref<_PropertySnapshot> snapshot = _history->get_snapshot(offset_tick);
		for(int i = 0; i < properties.size(); ++i)
		{
			Ref<PropertyEntry> property = properties[i];
			data.append(snapshot->get_value(property->_to_string()));
		}
	}

	data.append(_version);
	return data;
}

TypedArray<_PropertySnapshot> _RedundantHistoryEncoder::decode(Array data, TypedArray<PropertyEntry> properties)
{
	if(data.is_empty() || properties.is_empty())
		return Array();

	int packet_version = data.pop_back();

	if(packet_version != _version)
	{
		if(!_has_received)
		{
			// First packet, assume version is OK
			_version = packet_version;
		}
		else
		{
			// Version mismatch, can't parse
			_logger->warning(vformat("Version mismatch! own: %d, received: %s", _version, packet_version));
			return Array();
		}
	}

	TypedArray<_PropertySnapshot> result = Array();
	int redundancy = data.size() / properties.size();
	for (int i = 0; i < redundancy; ++i)
		result.append(_PropertySnapshot::new_());
	
	for(int i=0; i<data.size(); i+=1)
	{
		Variant offset_idx = i / properties.size();
		Variant prop_idx = i % properties.size();

		Ref<_PropertySnapshot> snapshot = result[offset_idx];
		Ref<PropertyEntry> property_entry = properties[prop_idx];
		snapshot->set_value(property_entry->_to_string(), data[i]);
	}

	_has_received = true;
	return result;
}

int _RedundantHistoryEncoder::apply(int tick, TypedArray<_PropertySnapshot> snapshots, int sender)
{
	int earliest_new_tick = -1;

	for(int i=0; i<snapshots.size(); i+=1)
	{
		int offset_tick = tick - i;
		Ref<_PropertySnapshot> snapshot = snapshots[i];

		if(offset_tick < (int) Utils::get_autoload("NetworkRollback")->get("history_start"))
		{
			// Data too old
			_logger->warning(vformat("Received data for %s, rejecting because older than %s frames", offset_tick, Utils::get_autoload("NetworkRollback")->get("history_limit")));
			continue;
		}

		if(sender > 0)
		{
			snapshot->sanitize(sender, _property_cache);
			if(snapshot->is_empty())
			{
				// No valid properties ( probably after sanitize )
				_logger->warning(vformat("Received invalid data from %d for tick %d", sender, tick));
				continue;
			}
		}

		Ref<_PropertySnapshot> known_snapshot = _history->get_snapshot(offset_tick);
		if(!known_snapshot->equals(snapshot))
		{
			// Received a new snapshot, store and emit signal
			_history->set_snapshot(offset_tick, snapshot);
			earliest_new_tick = offset_tick;
		}
	}

	return earliest_new_tick;
}

Ref<_RedundantHistoryEncoder> _RedundantHistoryEncoder::new_(Ref<_PropertyHistoryBuffer> p_history, Ref<PropertyCache> p_property_cache)
{
	Ref<_RedundantHistoryEncoder> ref;
	ref.instantiate();
	ref->_history = p_history;
	ref->_property_cache = p_property_cache;
	return ref;
}

Ref<_NetfoxLogger> _RedundantHistoryEncoder::_logger;

void _RedundantHistoryEncoder::_bind_methods()
{
	_logger = _NetfoxLogger::for_netfox("_RedundantHistoryEncoder");

	ClassDB::bind_static_method("_RedundantHistoryEncoder", D_METHOD("new_", "p_history", "p_property_cache"), &_RedundantHistoryEncoder::new_);
	ClassDB::bind_method(D_METHOD("get_redundancy"), &_RedundantHistoryEncoder::get_redundancy);
	ClassDB::bind_method(D_METHOD("set_redundancy", "p_redundancy"), &_RedundantHistoryEncoder::set_redundancy);
	ClassDB::bind_method(D_METHOD("set_properties", "properties"), &_RedundantHistoryEncoder::set_properties);
	ClassDB::bind_method(D_METHOD("encode", "tick", "properties"), &_RedundantHistoryEncoder::encode);
	ClassDB::bind_method(D_METHOD("decode", "data", "properties"), &_RedundantHistoryEncoder::decode);
	ClassDB::bind_method(D_METHOD("apply", "tick", "snapshots", "sender"), &_RedundantHistoryEncoder::apply, DEFVAL(0));
	
	ADD_PROPERTY(PropertyInfo(Variant::INT, "redundancy"), "set_redundancy", "get_redundancy");
}

