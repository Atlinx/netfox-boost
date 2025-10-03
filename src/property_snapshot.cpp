
#include "property_snapshot.h"

#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

Dictionary _PropertySnapshot::as_dictionary()
{
	return _snapshot.duplicate();
}

Ref<_PropertySnapshot> _PropertySnapshot::from_dictionary(Dictionary data)
{
	return _new(data);
}

Ref<_PropertySnapshot> _PropertySnapshot::_new(Dictionary p_snapshot)
{
	Ref<_PropertySnapshot> ref;
	ref.instantiate();
	ref->_snapshot = p_snapshot;
	return ref;
}

void _PropertySnapshot::set_value(String property_path, Variant data)
{
	_snapshot[property_path] = data;
}

Variant _PropertySnapshot::get_value(String property_path)
{
	return _snapshot.get(property_path, Variant());
}

Array _PropertySnapshot::properties()
{
	return _snapshot.keys();
}

bool _PropertySnapshot::has(String property_path)
{
	return _snapshot.has(property_path);
}

int _PropertySnapshot::size()
{
	return _snapshot.size();
}

bool _PropertySnapshot::equals(Ref<_PropertySnapshot> other)
{
	return _snapshot == other->_snapshot;
}

bool _PropertySnapshot::is_empty()
{
	return _snapshot.is_empty();
}

void _PropertySnapshot::apply(Ref<PropertyCache> cache)
{
	Array keys = _snapshot.keys();
	for(int i = 0; i < keys.size(); ++i)
	{
		String property_path = keys[i];
		Ref<PropertyEntry> property_entry = cache->get_entry(property_path);
		Variant value = _snapshot[property_path];
		property_entry->set_value(value);
	}
}

Ref<_PropertySnapshot> _PropertySnapshot::merge(Ref<_PropertySnapshot> data)
{
	Dictionary result = _snapshot.duplicate();
	Dictionary data_dict = data->as_dictionary();
	Array keys = data_dict.keys();
	for(int i = 0; i < keys.size(); ++i)
	{
		String key = data_dict[i];
		result[key] = data->_snapshot[key];
	}

	return from_dictionary(result);
}

Ref<_PropertySnapshot> _PropertySnapshot::make_patch(Ref<_PropertySnapshot> data)
{
	Dictionary result = Dictionary();

	Array keys = data->properties();
	for(int i = 0; i < keys.size(); ++i)
	{
		String property_path = keys[i];
		Variant old_property = get_value(property_path);
		Variant new_property = data->get_value(property_path);

		if(old_property != new_property)
			result[property_path] = new_property;
	}

	return from_dictionary(result);
}

void _PropertySnapshot::sanitize(int sender, Ref<PropertyCache> property_cache)
{
	Dictionary sanitized = Dictionary();

	Array keys = _snapshot.keys();
	for(int i = 0; i < keys.size(); ++i)
	{
		Variant property = keys[i];
		Ref<PropertyEntry> property_entry = property_cache->get_entry(property);
		int authority = property_entry->node->get_multiplayer_authority();

		if(authority == sender)
			sanitized[property] = _snapshot[property];
		else
			_logger->warning(vformat("Received data for property %s, owned by %s, from sender %s", property, authority, sender));
	}

	_snapshot = sanitized;
}

Ref<_PropertySnapshot> _PropertySnapshot::extract(Array properties)
{
	Dictionary result = Dictionary();
	for(int i = 0; i < properties.size(); ++i)
	{
		Ref<PropertyEntry> property = properties[i];
		result[property->to_string()] = property->get_value();
	}
	return from_dictionary(result);
}

void _PropertySnapshot::_bind_methods() {
	ClassDB::bind_static_method("_PropertySnapshot", D_METHOD("as_dictionary"), &_PropertySnapshot::as_dictionary);
	ClassDB::bind_static_method("_PropertySnapshot", D_METHOD("new", "p_snapshot"), &_PropertySnapshot::_new);
	ClassDB::bind_static_method("_PropertySnapshot", D_METHOD("extract", "properties"), &_PropertySnapshot::extract);
	
	ClassDB::bind_method(D_METHOD("set_value", "property_path", "data"), &_PropertySnapshot::set_value);
	ClassDB::bind_method(D_METHOD("get_value", "property_path"), &_PropertySnapshot::get_value);
	ClassDB::bind_method(D_METHOD("properties"), &_PropertySnapshot::properties);
	ClassDB::bind_method(D_METHOD("has", "property_path"), &_PropertySnapshot::has);
	ClassDB::bind_method(D_METHOD("size"), &_PropertySnapshot::size);
	ClassDB::bind_method(D_METHOD("equals", "other"), &_PropertySnapshot::equals);
	ClassDB::bind_method(D_METHOD("is_empty"), &_PropertySnapshot::is_empty);
	ClassDB::bind_method(D_METHOD("apply", "cache"), &_PropertySnapshot::apply);
	ClassDB::bind_method(D_METHOD("merge", "data"), &_PropertySnapshot::merge);
	ClassDB::bind_method(D_METHOD("make_patch", "data"), &_PropertySnapshot::make_patch);
	ClassDB::bind_method(D_METHOD("sanitize", "sender", "property_cache"), &_PropertySnapshot::sanitize);

}

