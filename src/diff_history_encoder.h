#pragma once

#include "property_cache.h"
#include "property_entry.h"
#include "logger.h"
#include "property_history_buffer.h"
#include "property_snapshot.h"
#include "internal_bimap.h"

#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

using namespace godot;

class _DiffHistoryEncoder : public RefCounted {
	GDCLASS(_DiffHistoryEncoder, RefCounted);
public:

protected:
	Ref<_PropertyHistoryBuffer> _history;
	Ref<PropertyCache> _property_cache;

	Dictionary _full_snapshot;
	Dictionary _encoded_snapshot;

	InternalBiMap<int, String> _property_indexes;

	uint8_t _version = 0;
	bool _has_received = false;

	static Ref<_NetfoxLogger> _logger;
	static void _bind_methods();

public:
	_DiffHistoryEncoder() = default;
	~_DiffHistoryEncoder() override = default;

	static Ref<_DiffHistoryEncoder> new_(Ref<_PropertyHistoryBuffer> p_history, Ref<PropertyCache> p_property_cache);

	void add_properties(TypedArray<PropertyEntry> properties);
	PackedByteArray encode(int tick, int reference_tick, TypedArray<PropertyEntry> properties);
	Ref<_PropertySnapshot> decode(PackedByteArray data, TypedArray<PropertyEntry> properties);

// TODO: Rework metrics so these are not needed
	bool apply(int tick, Ref<_PropertySnapshot> snapshot, int reference_tick, int sender = -1);
	Dictionary get_encoded_snapshot();
	Dictionary get_full_snapshot();
	bool _ensure_property_idx(String property);
};