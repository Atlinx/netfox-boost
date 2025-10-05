#pragma once

#include "property_cache.h"
#include "property_entry.h"
#include "property_config.h"
#include "property_history_buffer.h"
#include "property_snapshot.h"
#include "set.h"

#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/templates/hash_set.hpp>

using namespace godot;

class _RollbackHistoryRecorder : public RefCounted {
	GDCLASS(_RollbackHistoryRecorder, RefCounted);
public:

// Provided externally by RBS

protected:
	Ref<_PropertyHistoryBuffer> _state_history;
	Ref<_PropertyHistoryBuffer> _input_history;

	Ref<_PropertyConfig> _state_property_config;
	Ref<_PropertyConfig> _input_property_config;

	Ref<PropertyCache> _property_cache;

	int _latest_state_tick;
	Ref<_Set> _skipset;

	static void _bind_methods();

public:
	_RollbackHistoryRecorder() = default;
	~_RollbackHistoryRecorder() override = default;

	void configure(Ref<_PropertyHistoryBuffer> p_state_history, Ref<_PropertyHistoryBuffer> p_input_history, Ref<_PropertyConfig> p_state_property_config, Ref<_PropertyConfig> p_input_property_config, Ref<PropertyCache> p_property_cache, Ref<_Set> p_skipset);

	void set_latest_state_tick(int p_latest_state_tick);
	void apply_state(int tick);
	void apply_display_state();
	void apply_tick(int tick);
	void trim_history();
	void record_input(int tick);
	void record_state(int tick);

	bool _should_record_tick(int tick);
	TypedArray<PropertyEntry> _get_state_props_to_record(int tick);

// Shared utils, extract later
	bool _should_record_property(Ref<PropertyEntry> property_entry, int tick);
	TypedArray<PropertyEntry> _get_recorded_state_props();
	TypedArray<PropertyEntry> _get_owned_state_props();
	TypedArray<PropertyEntry> _get_recorded_input_props();
	TypedArray<PropertyEntry> _get_owned_input_props();
};