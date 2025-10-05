#pragma once

#include "property_cache.h"
#include "property_entry.h"
#include "property_snapshot.h"
#include "logger.h"

#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

using namespace godot;

class _PropertySnapshot : public RefCounted {
	GDCLASS(_PropertySnapshot, RefCounted);
public:

// Maps property paths to their values
// Dictionary[String, Variant]

protected:
	Dictionary _snapshot;

	static Ref<_NetfoxLogger> _logger;
	static void _bind_methods();

public:
	_PropertySnapshot() = default;
	~_PropertySnapshot() override = default;

	static Ref<_PropertySnapshot> from_dictionary(Dictionary data);
	static Ref<_PropertySnapshot> extract(TypedArray<PropertyEntry> properties);
	static Ref<_PropertySnapshot> new_(Dictionary p_snapshot = Dictionary());
	
	Dictionary as_dictionary();
	void set_value(String property_path, Variant data);
	Variant get_value(String property_path);
	TypedArray<String> properties();
	bool has(String property_path);
	int size();
	bool equals(Ref<_PropertySnapshot> other);
	bool is_empty();
	void apply(Ref<PropertyCache> cache);
	Ref<_PropertySnapshot> merge(Ref<_PropertySnapshot> data);
	Ref<_PropertySnapshot> make_patch(Ref<_PropertySnapshot> data);
	void sanitize(int sender, Ref<PropertyCache> property_cache);
};
