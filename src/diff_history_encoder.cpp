
#include "diff_history_encoder.h"
#include "utils.h"

#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/stream_peer_buffer.hpp>

Ref<_DiffHistoryEncoder> _DiffHistoryEncoder::new_(Ref<_PropertyHistoryBuffer> p_history, Ref<PropertyCache> p_property_cache)
{	
	Ref<_DiffHistoryEncoder> ref;
	ref.instantiate();
	ref->_history = p_history;
	ref->_property_cache = p_property_cache;
	return ref;
}

void _DiffHistoryEncoder::add_properties(TypedArray<PropertyEntry> properties)
{
	bool has_new_properties = false;

	for(Ref<PropertyEntry> property_entry : properties)
	{
		bool is_new = _ensure_property_idx(property_entry->_to_string());
		has_new_properties = has_new_properties || is_new;
	}

	// If we added any new properties, increment version
	if(has_new_properties)
		_version = (_version + 1) % 256;
}

PackedByteArray _DiffHistoryEncoder::encode(int tick, int reference_tick, TypedArray<PropertyEntry> properties)
{
	ERR_FAIL_COND_V_MSG(properties.size() > 255, PackedByteArray(), "Property indices may not fit into bytes!");

	Ref<_PropertySnapshot> snapshot = _history->get_snapshot(tick);
	Ref<_PropertySnapshot> reference_snapshot = _history->get_history(reference_tick);
	Ref<_PropertySnapshot> diff_snapshot = reference_snapshot->make_patch(snapshot);

	_full_snapshot = snapshot->as_dictionary();
	_encoded_snapshot = diff_snapshot->as_dictionary();

	if(diff_snapshot->is_empty())
		return PackedByteArray();

	Ref<StreamPeerBuffer> buffer;
	buffer.instantiate();
	buffer->put_u8(_version);

	for(String property : diff_snapshot->properties())
	{
		int property_idx = _property_indexes.get_by_value(property);
		Variant property_value = diff_snapshot->get_value(property);

		buffer->put_u8(property_idx);
		buffer->put_var(property_value);
	}

	return buffer->get_data_array();
}

Ref<_PropertySnapshot> _DiffHistoryEncoder::decode(PackedByteArray data, TypedArray<PropertyEntry> properties)
{
	Ref<_PropertySnapshot> result = _PropertySnapshot::new_();

	if(data.is_empty())
		return result;

	Ref<StreamPeerBuffer> buffer;
	buffer.instantiate();
	buffer->set_data_array(data);

	uint8_t packet_version = buffer->get_u8();
	if(packet_version != _version)
	{
		if(!_has_received)
		{
			// This is the first time we receive data
			// Assume the version is OK
			_version = packet_version;
		}
		else
		{
			// Since we don't remove entries, only add, we can still parse what we can
			_logger()->warning(vformat("Property config version mismatch - own %d != received %d", _version, packet_version));
		}
	}

	_has_received = true;

	while(buffer->get_available_bytes() > 0)
	{
		int property_idx = buffer->get_u8();
		Variant property_value = buffer->get_var();
		if(!_property_indexes.has_key(property_idx))
		{
			_logger()->warning(vformat("Received unknown property index %d, ignoring!", property_idx));
			continue;
		}

		Variant property_entry = _property_indexes.get_by_key(property_idx);
		result->set_value(property_entry, property_value);
	}

	return result;
}

bool _DiffHistoryEncoder::apply(int tick, Ref<_PropertySnapshot> snapshot, int reference_tick, int sender)
{
	if(tick < (int) Utils::get_autoload("NetworkRollback")->get("history_start"))
	{
		// State too old!
		_logger()->error(vformat("Received diff snapshot for @%d, rejecting because older than %s frames", tick, Utils::get_autoload("NetworkRollback")->get("history_limit")));
		return false;
	}

	if(snapshot->is_empty())
		return true;

	if(sender > 0)
	{
		snapshot->sanitize(sender, _property_cache);
		if(snapshot->is_empty())
		{
			_logger()->warning(vformat("Received invalid diff from #%s for @%s", sender, tick));
			return false;
		}
	}

	if(!_history->has(reference_tick))
	{
		// Reference tick missing, hope for the best
		_logger()->warning(vformat("Reference tick %d missing for #%s applying %d", reference_tick, sender, tick));
	}

	Ref<_PropertySnapshot> reference_snapshot = _history->get_snapshot(reference_tick);
	_history->set_snapshot(tick, reference_snapshot->merge(snapshot));
	return true;
}

Dictionary _DiffHistoryEncoder::get_encoded_snapshot()
{
	return _encoded_snapshot;
}

Dictionary _DiffHistoryEncoder::get_full_snapshot()
{
	return _full_snapshot;
}

bool _DiffHistoryEncoder::_ensure_property_idx(String property)
{
	if(_property_indexes.has_value(property))
	{
		return false;
	}

	ERR_FAIL_COND_V_MSG(_property_indexes.size() >= 256, false, "Property index map is full, can't add new property!");
	int idx = UtilityFunctions::hash(property) % 256;
	while(_property_indexes.has_key(idx))
	{
		idx = UtilityFunctions::hash(idx + 1) % 256;
	}
	_property_indexes.insert(idx, property);

	return true;
}

void _DiffHistoryEncoder::_bind_methods() 
{
	ClassDB::bind_static_method("_DiffHistoryEncoder", D_METHOD("new_", "p_history", "p_property_cache"), &_DiffHistoryEncoder::new_);
	ClassDB::bind_method(D_METHOD("add_properties", "properties"), &_DiffHistoryEncoder::add_properties);
	ClassDB::bind_method(D_METHOD("encode", "tick", "reference_tick", "properties"), &_DiffHistoryEncoder::encode);
	ClassDB::bind_method(D_METHOD("decode", "data", "properties"), &_DiffHistoryEncoder::decode);
	ClassDB::bind_method(D_METHOD("apply", "tick", "snapshot", "reference_tick", "sender"), &_DiffHistoryEncoder::apply, DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("get_encoded_snapshot"), &_DiffHistoryEncoder::get_encoded_snapshot);
	ClassDB::bind_method(D_METHOD("get_full_snapshot"), &_DiffHistoryEncoder::get_full_snapshot);

}

