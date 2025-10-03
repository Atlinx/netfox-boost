#pragma once

#include "property_cache.h"
#include "logger.h"
#include "property_history_buffer.h"
#include "property_snapshot.h"

#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

using namespace godot;

class _SnapshotHistoryEncoder : public RefCounted {
	GDCLASS(_SnapshotHistoryEncoder, RefCounted);
public:

protected:
	Ref<_PropertyHistoryBuffer> _history;
	Ref<PropertyCache> _property_cache;
	Array _properties;

	uint8_t _version = -1;
	bool _has_received = false;

	static Ref<_NetfoxLogger> _logger;
	static void _bind_methods();

public:
	_SnapshotHistoryEncoder() = default;
	~_SnapshotHistoryEncoder() override = default;

	static void _static_init();
	static Ref<_SnapshotHistoryEncoder> _new(Ref<_PropertyHistoryBuffer> p_history, Ref<PropertyCache> p_property_cache);

	void set_properties(Array properties);
	Array encode(int tick, TypedArray<PropertyEntry> properties);
	Ref<_PropertySnapshot> decode(Array data, TypedArray<PropertyEntry> properties);
	bool apply(int tick, Ref<_PropertySnapshot> snapshot, int sender =  - 1);
};