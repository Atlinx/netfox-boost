#pragma once

#include "logger.h"
#include "property_cache.h"
#include "property_entry.h"
#include "property_history_buffer.h"
#include "property_snapshot.h"

#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

using namespace godot;

class _RedundantHistoryEncoder : public RefCounted {
	GDCLASS(_RedundantHistoryEncoder, RefCounted);
public:

protected:
	int _redundancy = 4;
	Ref<_PropertyHistoryBuffer> _history;
	Array _properties;
	Ref<PropertyCache> _property_cache;

	int _version = 0;
	bool _has_received = false;

	static Ref<_NetfoxLogger> _logger;
	static void _bind_methods();

public:
	_RedundantHistoryEncoder() = default;
	~_RedundantHistoryEncoder() override = default;

	static void _static_init();
	static Ref<_RedundantHistoryEncoder> _new(Ref<_PropertyHistoryBuffer> p_history, Ref<PropertyCache> p_property_cache);

	int get_redundancy();
	void set_redundancy(int p_redundancy);
	void set_properties(TypedArray<PropertyEntry> properties);
	Array encode(int tick, TypedArray<PropertyEntry> properties);
	TypedArray<_PropertySnapshot> decode(Array data, TypedArray<PropertyEntry> properties);
	int apply(int tick, TypedArray<_PropertySnapshot> snapshots, int sender = 0);
};