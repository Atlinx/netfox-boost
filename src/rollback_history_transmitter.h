#pragma once

#include "peer_visibility_filter.h"
#include "property_cache.h"
#include "property_entry.h"
#include "logger.h"
#include "property_config.h"
#include "property_history_buffer.h"
#include "redundant_history_encoder.h"
#include "snapshot_history_encoder.h"
#include "diff_history_encoder.h"
#include "set.h"

#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/templates/hash_set.hpp>

using namespace godot;

class _RollbackHistoryTransmitter : public Node {
	GDCLASS(_RollbackHistoryTransmitter, Node);

protected:
	Node* root;
	bool enable_input_broadcast = true;
	int full_state_interval;
	int diff_ack_interval;

// Provided externally by RBS
	Ref<_PropertyHistoryBuffer> _state_history;
	Ref<_PropertyHistoryBuffer> _input_history;
	PeerVisibilityFilter* _visibility_filter;

	Ref<_PropertyConfig> _state_property_config;
	Ref<_PropertyConfig> _input_property_config;

	Ref<PropertyCache> _property_cache;
	Ref<_Set> _skipset;

// Collaborators
	Ref<_RedundantHistoryEncoder> _input_encoder;
	Ref<_SnapshotHistoryEncoder> _full_state_encoder;
	Ref<_DiffHistoryEncoder> _diff_state_encoder;

// State
	Dictionary _ackd_state = Dictionary();
	int _next_full_state_tick;
	int _next_diff_ack_tick;

	int _earliest_input_tick;
	int _latest_state_tick;

	bool _is_predicted_tick;
	bool _is_initialized;

// Signals
	/* signal _on_transmit_state(Dictionary state, int tick) */

	static Ref<_NetfoxLogger> _logger;
	static void _bind_methods();

public:
	_RollbackHistoryTransmitter();
	~_RollbackHistoryTransmitter() override = default;

	int get_earliest_input_tick();
	int get_latest_state_tick();
	void set_predicted_tick(Variant p_is_predicted_tick);
	void sync_settings(Node* p_root, bool p_enable_input_broadcast, int p_full_state_interval, int p_diff_ack_interval);
	void configure(Ref<_PropertyHistoryBuffer> p_state_history, Ref<_PropertyHistoryBuffer> p_input_history, Ref<_PropertyConfig> p_state_property_config, Ref<_PropertyConfig> p_input_property_config, PeerVisibilityFilter* p_visibility_filter, Ref<PropertyCache> p_property_cache, Ref<_Set> p_skipset);
	void reset();
	void conclude_tick_loop();
	void transmit_input(int tick);
	void transmit_state(int tick);
	bool _should_broadcast(Ref<PropertyEntry> property, int tick);
	void _send_full_state(int tick, int peer = 0);

	// `serialized_state` is a serialized _PropertySnapshot
	void _submit_input(int tick, Array data);
	// State is a serialized _PropertySnapshot (Dictionary[String, Variant])
	void _submit_full_state(Array data, int tick);
	void _submit_diff_state(PackedByteArray data, int tick, int reference_tick);
	void _ack_full_state(int tick);

	// Shared utils, extract later
	void _ack_diff_state(int tick);
	TypedArray<PropertyEntry> _get_recorded_state_props();
	TypedArray<PropertyEntry> _get_owned_state_props();
	TypedArray<PropertyEntry> _get_recorded_input_props();
	TypedArray<PropertyEntry> _get_owned_input_props();
};