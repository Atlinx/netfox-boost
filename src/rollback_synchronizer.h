#pragma once

#include "logger.h"
#include "property_config.h"
#include "rollback_history_recorder.h"
#include "rollback_history_transmitter.h"

#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/classes/node.hpp>

using namespace godot;

class RollbackSynchronizer : public Node {
	GDCLASS(RollbackSynchronizer, Node);
public:

protected:
	Node* root = get_parent();
	bool enable_prediction = false;
	Array state_properties;

//# Ticks to wait between sending full states.
//# [br][br]
//# If set to 0, full states will never be sent. If set to 1, only full states
//# will be sent. If set higher, full states will be sent regularly, but not
//# for every tick.
//# [br][br]
//# Only considered if [member _NetworkRollback.enable_diff_states] is true.

	int full_state_interval = 24;

//# Ticks to wait between unreliably acknowledging diff states.
//# [br][br]
//# This can reduce the amount of properties sent in diff states, due to clients
//# more often acknowledging received states. To avoid introducing hickups, these
//# are sent unreliably.
//# [br][br]
//# If set to 0, diff states will never be acknowledged. If set to 1, all diff
//# states will be acknowledged. If set higher, ack's will be sent regularly, but
//# not for every diff state.
//# [br][br]
//# If enabled, it's worth to tune this setting until network traffic is actually
//# reduced.
//# [br][br]
//# Only considered if [member _NetworkRollback.enable_diff_states] is true.

	int diff_ack_interval = 0;

//# Properties that define the input for the game simulation.
//# [br][br]
//# Input properties drive the simulation, which in turn results in updated state
//# properties. Input is recorded after every network tick.
	Array input_properties;

//# This will broadcast input to all peers, turning this off will limit to
//# sending it to the server only. Turning this off is recommended to save
//# bandwidth and reduce cheating risks.
	bool enable_input_broadcast = true;

// Make sure this exists from the get-go, just not in the scene tree
//# Decides which peers will receive updates
	Variant visibility_filter = PeerVisibilityFilter->new();

	Ref<_PropertyConfig> _state_property_config = _PropertyConfig->new();
	Ref<_PropertyConfig> _input_property_config = _PropertyConfig->new();
	Array _nodes = Array();

	Ref<_Set> _simset = _Set.new();
	Ref<_Set> _skipset = _Set.new();

	bool _properties_dirty = false;

	Variant _property_cache = PropertyCache->new(root);
	Variant _freshness_store = RollbackFreshnessStore.new();

	Variant _states = _PropertyHistoryBuffer->new();
	Variant _inputs = _PropertyHistoryBuffer->new();
	int _last_simulated_tick;

	bool _has_input;
	int _input_tick;
	bool _is_predicted_tick;

	static Ref<_NetfoxLogger> _logger;

// Composition
	Ref<_RollbackHistoryTransmitter> _history_transmitter;
	Ref<_RollbackHistoryRecorder> _history_recorder;

//# Process settings.
//#
//# Call this after any change to configuration. Updates based on authority too
//# ( calls process_authority ).
//# peers.

public:
	void process_settings();
//# property is already tracked, this method does nothing.
	void process_authority();
//# property is already tracked, this method does nothing.
	void add_state(Variant node, String property);
//# Returns true if input is available.
	void add_input(Variant node, String property);
//# Calling this when [member has_input] is false will yield an error.
	bool has_input();
//# authoritative.
	int get_input_age();
//# ignored if [member enable_prediction] is false.
	bool is_predicting();
//# Returns -1 if there's no known input.
	void ignore_prediction(Node* node);
//# will be the tick of the latest state received from the state owner.
	int get_last_known_input();

	int get_last_known_state();

	void _ready() override;

	void _connect_signals();

	void _disconnect_signals();

	void _before_tick(double _dt, int tick);

	void _after_tick(double _dt, int tick);

	void _before_rollback_loop();

	void _on_prepare_tick(int tick);

	void _process_tick(int tick);

	void _on_record_tick(int tick);

	void _after_rollback_loop();

	void _notification(int what) override;

	PackedStringArray _get_configuration_warnings() override;

	void _enter_tree() override;

	void _exit_tree() override;

	void _notify_resim();

	void _prepare_tick_process(int tick);

// `node` can be set to null, in case we're not simulating a specific node
	bool _can_simulate(Node* node, int tick);

	bool _is_predicted_tick_for(Node* node, int tick);

	void _run_rollback_tick(int tick);

	void _push_simset_metrics();

	void _reprocess_settings();

	Array _get_recorded_state_props();

	Array _get_owned_state_props();

	Array _get_recorded_input_props();

	Array _get_owned_input_props();
	void set_root(Node* value);
	Node* get_root();
	void set_enable_prediction(bool value);
	bool get_enable_prediction();
	void set_state_properties(Array value);
	Array get_state_properties();
	void set_full_state_interval(int value);
	int get_full_state_interval();
	void set_diff_ack_interval(int value);
	int get_diff_ack_interval();
	void set_input_properties(Array value);
	Array get_input_properties();
	void set_enable_input_broadcast(bool value);
	bool get_enable_input_broadcast();

	static void _bind_methods();
};