#pragma once

#include "property_entry.h"

#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/array.hpp>

using namespace godot;

class PropertyCache : public RefCounted {
	GDCLASS(PropertyCache, RefCounted);
public:

protected:
	Node* root;
	HashMap<String, Ref<PropertyEntry>> _cache;

	static Ref<_NetfoxLogger> _logger;
	static void _bind_methods();
	
public:
	PropertyCache() = default;
	~PropertyCache() override = default;

	Ref<PropertyEntry> get_entry(String path);
	TypedArray<PropertyEntry> gd_properties();
	Vector<Ref<PropertyEntry>> properties();
	void clear();

	static Ref<PropertyCache> new_(Node* p_root);
};