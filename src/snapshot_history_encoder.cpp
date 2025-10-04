
#include "snapshot_history_encoder.h"
#include "utils.h"

#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

Ref<_NetfoxLogger> _SnapshotHistoryEncoder::_logger;

Ref<_SnapshotHistoryEncoder> _SnapshotHistoryEncoder::new_(Ref<_PropertyHistoryBuffer> p_history, Ref<PropertyCache> p_property_cache)
{	
	Ref<_SnapshotHistoryEncoder> ref;
	ref.instantiate();
	ref->_history = p_history;
	ref->_property_cache = p_property_cache;
	return ref;
}

void _SnapshotHistoryEncoder::set_properties(Array properties)
{
	if(_properties != properties)
	{
		_version = (_version + 1) % 256;
		_properties = properties.duplicate();
	}
}

Array _SnapshotHistoryEncoder::encode(int tick, TypedArray<PropertyEntry> properties)
{
	Ref<_PropertySnapshot> snapshot = _history->get_snapshot(tick);
	Array data = Array();
	data.resize(properties.size());

	for(int i=0; i<properties.size(); i+=1)
	{
		Ref<PropertyEntry> property_entry = properties[i];
		data[i] = snapshot->get_value(property_entry->to_string());
	}
	data.append(_version);

	return data;
}

Ref<_PropertySnapshot> _SnapshotHistoryEncoder::decode(Array data, TypedArray<PropertyEntry> properties)
{
	Ref<_PropertySnapshot> result = _PropertySnapshot::new_();
	uint8_t packet_version = data.pop_back();

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
			return result;
		}
	}

	if(properties.size() != data.size())
	{
		_logger->warning(vformat("Received snapshot with %d entries, with %d known - parsing as much as possible", data.size(), properties.size()));
	}

	for(int i=0; i< Math::min(data.size(), properties.size()); i+=1)
	{
		Ref<PropertyEntry> entry = properties[i];
		result->set_value(entry->to_string(), data[i]);
	}

	_has_received = true;
	return result;
}

bool _SnapshotHistoryEncoder::apply(int tick, Ref<_PropertySnapshot> snapshot, int sender)
{
	auto network_rollback = Utils::get_autoload("NetworkRollback");
	if(tick < (int) network_rollback->get("history_start"))
	{
		// State too old!
		_logger->error(vformat("Received full snapshot for %s, rejecting because older than %s frames", tick, network_rollback->get("history_limit")));
		return false;
	}

	if(sender > 0)
	{
		snapshot->sanitize(sender, _property_cache);
		if(snapshot->is_empty())
		{return false;
		}
	}

	_history->set_snapshot(tick, snapshot);
	return true;
}

void _SnapshotHistoryEncoder::_bind_methods() 
{
	_logger = _NetfoxLogger::for_netfox("_SnapshotHistoryEncoder");

	ClassDB::bind_static_method("_SnapshotHistoryEncoder", D_METHOD("new_", "p_history", "p_property_cache"), &_SnapshotHistoryEncoder::new_);
	ClassDB::bind_method(D_METHOD("set_properties", "properties"), &_SnapshotHistoryEncoder::set_properties);
	ClassDB::bind_method(D_METHOD("encode", "tick", "properties"), &_SnapshotHistoryEncoder::encode);
	ClassDB::bind_method(D_METHOD("decode", "data", "properties"), &_SnapshotHistoryEncoder::decode);
	ClassDB::bind_method(D_METHOD("apply", "tick", "snapshot", "sender"), &_SnapshotHistoryEncoder::apply, DEFVAL(-1));
}

