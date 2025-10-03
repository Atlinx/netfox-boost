
#include "property_cache.h"

#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

Ref<_NetfoxLogger> PropertyCache::_logger;

void PropertyCache::_static_init()
{
	_logger = _NetfoxLogger::for_netfox("PropertyCache");
}

Ref<PropertyCache> PropertyCache::_new(Node* p_root)
{
	if (_logger.is_null())
		_logger = _NetfoxLogger::for_netfox("PropertyCache");
	
	Ref<PropertyCache> ref;
	ref.instantiate();
	ref->root = p_root;
	return ref;
}

Ref<PropertyEntry> PropertyCache::get_entry(String path)
{
	if(!_cache.has(path))
	{
		Ref<PropertyEntry> parsed = PropertyEntry::parse(root, path);
		if(!parsed->is_valid())
		{
			_logger->warning(vformat("Invalid property path: %s", path));
		}
		_cache[path] = parsed;
	}
	return _cache[path];
}

TypedArray<PropertyEntry> PropertyCache::gd_properties()
{
	TypedArray<PropertyEntry> result;
	
	result.resize(_cache.size());
	int i = 0;
	for (auto key_value : _cache) {
		result[i] = key_value.value;
		++i;
	}

	return result;
}

Vector<Ref<PropertyEntry>> PropertyCache::properties()
{
	Vector<Ref<PropertyEntry>> result;
	result.resize(_cache.size());
	int i = 0;
	// Iterate over the HashMap and directly extract the Ref<PropertyEntry> values
	for (auto key_value : _cache) {
		result.set(i, key_value.value);
		++i;
	}
	return result;
}

void PropertyCache::clear()
{
	_cache.clear();
}

void PropertyCache::_bind_methods() {
	ClassDB::bind_static_method("PropertyCache", D_METHOD("new", "p_root"), &PropertyCache::_new);
	ClassDB::bind_method(D_METHOD("get_entry", "path"), &PropertyCache::get_entry);
	ClassDB::bind_method(D_METHOD("properties"), &PropertyCache::gd_properties);
	ClassDB::bind_method(D_METHOD("clear"), &PropertyCache::clear);
}

