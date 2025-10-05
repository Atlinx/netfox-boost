#pragma once

#include "property_cache.h"
#include "property_entry.h"

#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>

using namespace godot;

class _PropertyConfig : public RefCounted {
	GDCLASS(_PropertyConfig, RefCounted);
public:

protected:
	Vector<Ref<PropertyEntry>> _properties;
	HashMap<int, Vector<Ref<PropertyEntry>>> _auth_properties;

	int local_peer_id = 0; // Initialize to 0 (unassigned)

	static void _bind_methods();

public:
	_PropertyConfig() = default;
	~_PropertyConfig() override = default;

	void clear();

	void set_properties(TypedArray<PropertyEntry> p_properties);
	void set_properties_from_paths(Array property_paths, Ref<PropertyCache> property_cache);
	TypedArray<PropertyEntry> get_properties() const;
	TypedArray<PropertyEntry> get_owned_properties();
	TypedArray<PropertyEntry> get_properties_owned_by(int peer);

	void set_local_peer_id(int p_id);
	int get_local_peer_id() const;
};