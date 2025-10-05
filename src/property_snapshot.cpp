
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
	return new_(data);
}

Ref<_PropertySnapshot> _PropertySnapshot::new_(Dictionary p_snapshot)
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

TypedArray<String> _PropertySnapshot::properties()
{
	TypedArray<String> res;
	res.assign(_snapshot.keys());
	return res;
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
	// print_line("_PropertySnapshot::apply: cache: ", cache);
	for(String property_path : _snapshot.keys())
	{
		// print_line("  apply path: ", property_path);
		Ref<PropertyEntry> property_entry = cache->get_entry(property_path);
		// print_line("    property_entry: ", property_entry);
		Variant value = _snapshot[property_path];
		// print_line("    value: ", value);
		property_entry->set_value(value);
	}
}

Ref<_PropertySnapshot> _PropertySnapshot::merge(Ref<_PropertySnapshot> data)
{
	Dictionary result = _snapshot.duplicate();
	for (String key : data->_snapshot.keys())
		result[key] = data->_snapshot[key];

	return from_dictionary(result);
}

Ref<_PropertySnapshot> _PropertySnapshot::make_patch(Ref<_PropertySnapshot> data)
{
	Dictionary result;

	for (String property_path : data->properties())
	{
		Variant old_property = get_value(property_path);
		Variant new_property = data->get_value(property_path);

		if (old_property != new_property)
			result[property_path] = new_property;
	}

	return from_dictionary(result);
}

void _PropertySnapshot::sanitize(int sender, Ref<PropertyCache> property_cache)
{
	Dictionary sanitized;

	for (String property : _snapshot.keys())
	{
		Ref<PropertyEntry> property_entry = property_cache->get_entry(property);
		int authority = property_entry->node->get_multiplayer_authority();

		if (authority == sender)
			sanitized[property] = _snapshot[property];
		else
			_logger->warning(vformat("Received data for property %s, owned by %s, from sender %s", property, authority, sender));
	}

	_snapshot = sanitized;
}

Ref<_PropertySnapshot> _PropertySnapshot::extract(TypedArray<PropertyEntry> properties)
{
	Dictionary result;
	for (Ref<PropertyEntry> property : properties)
		result[property->_to_string()] = property->get_value();
	return from_dictionary(result);
}

Ref<_NetfoxLogger> _PropertySnapshot::_logger;

void _PropertySnapshot::_bind_methods()
{
	_logger = _NetfoxLogger::for_netfox("_PropertySnapshot");

	ClassDB::bind_static_method("_PropertySnapshot", D_METHOD("new_", "p_snapshot"), &_PropertySnapshot::new_);
	ClassDB::bind_static_method("_PropertySnapshot", D_METHOD("from_dictionary", "data"), &_PropertySnapshot::from_dictionary);
	ClassDB::bind_static_method("_PropertySnapshot", D_METHOD("extract", "properties"), &_PropertySnapshot::extract);
	
	ClassDB::bind_method(D_METHOD("as_dictionary"), &_PropertySnapshot::as_dictionary);
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

