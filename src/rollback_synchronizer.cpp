
#include "rollback_synchronizer.h"

#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

void RollbackSynchronizer::process_settings()
{
	_property_cache->root = root;
	_property_cache->clear();
	_freshness_store->clear();

	_nodes.clear();

	_states->clear();
	_inputs->clear();
	process_authority();

	// Gather all rollback-aware nodes to simulate during rollbacks
	_nodes = root->find_children("*");
	_nodes.push_front(root);
	_nodes = _nodes.filter([](Variant it) 
	{	return NetworkRollback.is_rollback_aware(it);;);
	}
	_nodes.erase(this);

	_history_transmitter->sync_settings(root, enable_input_broadcast, full_state_interval, diff_ack_interval);
	_history_transmitter->configure(_states, _inputs, _state_property_config, _input_property_config, visibility_filter, _property_cache, _skipset);
	_history_recorder->configure(_states, _inputs, _state_property_config, _input_property_config, _property_cache, _skipset);

	//# Process settings based on authority.
	//#
	//# Call this whenever the authority of any of the nodes managed by
	//# RollbackSynchronizer changes. Make sure to do this at the same time on all

}

void RollbackSynchronizer::process_authority()
{
	_state_property_config->set_local_peer_id(multiplayer->get_unique_id());
	_input_property_config->set_local_peer_id(multiplayer->get_unique_id());

	_state_property_config->set_properties_from_paths(state_properties, _property_cache);
	_input_property_config->set_properties_from_paths(input_properties, _property_cache);

	//# Add a state property.
	//# [br][br]
	//# Settings will be automatically updated. The [param node] may be a string or
	//# [NodePath] pointing to a node, or an actual [Node] instance. If the given

}

void RollbackSynchronizer::add_state(Variant node, String property)
{
	String property_path = PropertyEntry->make_path(root, node, property);
	if(!property_path || state_properties.has(property_path))
	{
		return ;
	}

	state_properties.push_back(property_path);
	_properties_dirty = true;
	_reprocess_settings.call_deferred();

	//# Add an input property.
	//# [br][br]
	//# Settings will be automatically updated. The [param node] may be a string or
	//# [NodePath] pointing to a node, or an actual [Node] instance. If the given

}

void RollbackSynchronizer::add_input(Variant node, String property)
{
	String property_path = PropertyEntry->make_path(root, node, property);
	if(!property_path || input_properties.has(property_path))
	{
		return ;
	}

	input_properties.push_back(property_path);
	_properties_dirty = true;
	_reprocess_settings.call_deferred();

	//# Check if input is available for the current tick.
	//#
	//# This input is not always current, it may be from multiple ticks ago.
	//# [br][br]

}

bool RollbackSynchronizer::has_input()
{
	return _has_input;

	//# Get the age of currently available input in ticks.
	//#
	//# The available input may be from the current tick, or from multiple ticks ago.
	//# This number of tick is the input's age.
	//# [br][br]

}

int RollbackSynchronizer::get_input_age()
{
	if(has_input())
	{
		return NetworkRollback.tick - _input_tick;
	}
	else
	{
		_logger->error("Trying to check input age without having input!");
		return  - 1;

		//# Check if the current tick is predicted.
		//#
		//# A tick becomes predicted if there's no up-to-date input available. It will be
		//# simulated and recorded, but will not be broadcast, nor considered

	}
}

bool RollbackSynchronizer::is_predicting()
{
	return _is_predicted_tick;

	//# Ignore a node's prediction for the current rollback tick.
	//#
	//# Call this when the input is too old to base predictions on. This call is

}

void RollbackSynchronizer::ignore_prediction(Node* node)
{
	if(enable_prediction)
	{
		_skipset->add(node);

		//# Get the tick of the last known input.
		//# [br][br]
		//# This is the latest tick where input information is available. If there's
		//# locally owned input for this instance ( e.g. running as client ), this value
		//# will be the current tick. Otherwise, this will be the latest tick received
		//# from the input owner.
		//# [br][br]
		//# If [member enable_input_broadcast] is false, there may be no input available
		//# for peers who own neither state nor input.
		//# [br][br]

	}
}

int RollbackSynchronizer::get_last_known_input()
{

	// If we own input, it is updated regularly, this will be the current tick
	// If we don't own input, _inputs is only updated when input data is received
	if(!_inputs->is_empty())
	{
		return _inputs->keys().Math::max();
	}
	return  - 1;

	//# Get the tick of the last known state.
	//# [br][br]
	//# This is the latest tick where information is available for state. For state
	//# owners ( usually the host ), this is the current tick. Note that even this
	//# data may change as new input arrives. For peers that don't own state, this

}

int RollbackSynchronizer::get_last_known_state()
{

	// If we own state, this will be updated when recording and broadcasting
	// state, this will be the current tick
	// If we don't own state, this will be updated when state data is received
	return _history_transmitter->get_latest_state_tick();
}

void RollbackSynchronizer::_ready()
{
	if(Engine::get_singleton()->is_editor_hint())
	{
		return ;
	}

	if(!NetworkTime.is_initial_sync_done())
	{

		// Wait for time sync to complete
		/* await NetworkTime->after_sync; */ // no equivalent to await in c++ !
	}

	process_settings.call_deferred();
}

void RollbackSynchronizer::_connect_signals()
{
	NetworkTime.before_tick.connect(_before_tick);
	NetworkTime.after_tick.connect(_after_tick);

	NetworkRollback.on_prepare_tick.connect(_on_prepare_tick);
	NetworkRollback.on_process_tick.connect(_process_tick);
	NetworkRollback.on_record_tick.connect(_on_record_tick);

	NetworkRollback.before_loop.connect(_before_rollback_loop);
	NetworkRollback.after_loop.connect(_after_rollback_loop);
}

void RollbackSynchronizer::_disconnect_signals()
{
	NetworkTime.before_tick.disconnect(_before_tick);
	NetworkTime.after_tick.disconnect(_after_tick);

	NetworkRollback.on_prepare_tick.disconnect(_on_prepare_tick);
	NetworkRollback.on_process_tick.disconnect(_process_tick);
	NetworkRollback.on_record_tick.disconnect(_on_record_tick);

	NetworkRollback.before_loop.disconnect(_before_rollback_loop);
	NetworkRollback.after_loop.disconnect(_after_rollback_loop);
}

void RollbackSynchronizer::_before_tick(double _dt, int tick)
{
	_history_recorder->apply_state(tick);
}

void RollbackSynchronizer::_after_tick(double _dt, int tick)
{
	_history_recorder->record_input(tick);
	_history_transmitter->transmit_input(tick);
	_history_recorder->trim_history();
	_freshness_store->trim();
}

void RollbackSynchronizer::_before_rollback_loop()
{
	_notify_resim();
}

void RollbackSynchronizer::_on_prepare_tick(int tick)
{
	_history_recorder->apply_tick(tick);
	_prepare_tick_process(tick);
}

void RollbackSynchronizer::_process_tick(int tick)
{
	_run_rollback_tick(tick);
	_push_simset_metrics();
}

void RollbackSynchronizer::_on_record_tick(int tick)
{
	_history_recorder->record_state(tick);
	_history_transmitter->transmit_state(tick);
}

void RollbackSynchronizer::_after_rollback_loop()
{
	_history_recorder->apply_display_state();
	_history_transmitter->conclude_tick_loop();
}

void RollbackSynchronizer::_notification(int what)
{
	if(what == NOTIFICATION_EDITOR_PRE_SAVE)
	{
		update_configuration_warnings();
	}
}

PackedStringArray RollbackSynchronizer::_get_configuration_warnings()
{
	if(!root)
	{
		root = get_parent();
	}

	// Explore state and input properties
	if(!root)
	{
		return Array {/* initializer lists are unsupported */ "No valid root node found!",  };
	}

	PackedStringArray result = PackedStringArray();

	result.append_array(_NetfoxEditorUtils.gather_properties(root, "_get_rollback_state_properties", [](Variant node, Variant prop) 
	{	add_state(node, prop);
	};
	));

	result.append_array(_NetfoxEditorUtils.gather_properties(root, "_get_rollback_input_properties", [](Variant node, Variant prop) 
	{	add_input(node, prop);
	};
	));

	return result;
}

void RollbackSynchronizer::_enter_tree()
{
	if(Engine::get_singleton()->is_editor_hint())
	{
		return ;
	}

	if(!visibility_filter)
	{
		visibility_filter = PeerVisibilityFilter->new();
	}

	if(!visibility_filter->get_parent())
	{
		add_child(visibility_filter);
	}

	if(_history_transmitter == null)
	{
		_history_transmitter = _RollbackHistoryTransmitter->new();
		add_child(_history_transmitter, true);
		_history_transmitter->set_multiplayer_authority(get_multiplayer_authority());
	}

	if(_history_recorder == null)
	{
		_history_recorder = _RollbackHistoryRecorder->new();
	}

	if(!NetworkTime.is_initial_sync_done())
	{

		// Wait for time sync to complete
		/* await NetworkTime->after_sync; */ // no equivalent to await in c++ !
	}
	_connect_signals.call_deferred();
	process_settings.call_deferred();
}

void RollbackSynchronizer::_exit_tree()
{
	if(Engine::get_singleton()->is_editor_hint())
	{
		return ;
	}

	_disconnect_signals();
}

void RollbackSynchronizer::_notify_resim()
{
	if(_get_owned_input_props().is_empty())
	{

		// We don't have any inputs we own, simulate from earliest we've received
		NetworkRollback.not;ify_resimulation_start(_history_transmitter->get_earliest_input_tick());
	}
	else
	{

		// We own inputs, simulate from latest authorative state
		NetworkRollback.not;ify_resimulation_start(_history_transmitter->get_latest_state_tick());
	}
}

void RollbackSynchronizer::_prepare_tick_process(int tick)
{
	_history_recorder->set_latest_state_tick(_history_transmitter->get__latest_state_tick());

	// Save data for input prediction
	Variant retrieved_tick = _inputs->get_closest_tick(tick);

	// These are used as input for input age ( i.e. do we even have input, and if so, how old? )
	_has_input = retrieved_tick !=  - 1;
	_input_tick = retrieved_tick;

	// Used to explicitly determine if this is a predicted tick
	// ( even if we could grab *some* input )
	_is_predicted_tick = _is_predicted_tick_for(null, tick);
	_history_transmitter->set_predicted_tick(_is_predicted_tick);

	// Reset the set of simulated and ignored nodes
	_simset->clear();
	_skipset->clear();

	// Gather nodes that can be simulated
	for(Node* node : _nodes)
	{
		if(_can_simulate(node, tick))
		{
			NetworkRollback.not;ify_simulated(node);
		}
	}
}

bool RollbackSynchronizer::_can_simulate(Node* node, int tick)
{
	if(!enable_prediction && _is_predicted_tick_for(node, tick))
	{

		// Don't simulate if prediction is not allowed and tick is predicted
		return false;
	}
	if(NetworkRollback.is_mutated(node, tick))
	{

		// Mutated nodes are always resimulated
		return true;
	}
	if(input_properties.is_empty())
	{

		// If we're running inputless and own the node, simulate it if we haven't
		if(node->is_multiplayer_authority())
		{
			return tick > _last_simulated_tick;
		}

		// If we're running inputless and don't own the node, only run as prediction
		return enable_prediction;
	}
	if(node->is_multiplayer_authority())
	{

		// Simulate from earliest input
		// Don't simulate frames we don't have input for
		return tick >= _history_transmitter->get_earliest_input_tick();
	}
	else
	{

		// Simulate ONLY if we have state from server
		// Simulate from latest authorative state - anything the server confirmed we don't rerun
		// Don't simulate frames we don't have input for
		return tick >= _history_transmitter->get_latest_state_tick();
	}
}

bool RollbackSynchronizer::_is_predicted_tick_for(Node* node, int tick)
{
	if(input_properties.is_empty() && node != null)
	{

		// We're running without inputs
		// It's only predicted if we don't own the node
		return !node->is_multiplayer_authority();
	}
	else
	{

		// We have input properties, it's only predicted if we don't have the input for the tick
		return !_inputs->has(tick);
	}
}

void RollbackSynchronizer::_run_rollback_tick(int tick)
{

	// Simulate rollback tick
	//	Method call on rewindables
	//	Rollback synchronizers go through each node they manage
	//	If current tick is in node's range, tick
	//		If authority: Latest input >= tick >= Latest state
	//		If not: Latest input >= tick >= Earliest input
	for(Node* node : _nodes)
	{
		if(!NetworkRollback.is_simulated(node))
		{
			continue;
		}

		Variant is_fresh = _freshness_store->is_fresh(node, tick);
		_is_predicted_tick = _is_predicted_tick_for(node, tick);
		NetworkRollback.process_rollback(node, NetworkTime.ticktime, tick, is_fresh);

		if(_skipset->has(node))
		{
			continue;
		}

		_freshness_store->not;ify_processed(node, tick);
		_simset->add(node);
	}
}

void RollbackSynchronizer::_push_simset_metrics()
{

	// Push metrics
	NetworkPerformance.push_rollback_nodes_simulated(_simset->size());
}

void RollbackSynchronizer::_reprocess_settings()
{
	if(!_properties_dirty || Engine::get_singleton()->is_editor_hint())
	{
		return ;
	}

	_properties_dirty = false;
	process_settings();
}

Array RollbackSynchronizer::_get_recorded_state_props()
{
	return _state_property_config->get_properties();
}

Array RollbackSynchronizer::_get_owned_state_props()
{
	return _state_property_config->get_owned_properties();
}

Array RollbackSynchronizer::_get_recorded_input_props()
{
	return _input_property_config->get_owned_properties();
}

Array RollbackSynchronizer::_get_owned_input_props()
{
	return _input_property_config->get_owned_properties();
}

void RollbackSynchronizer::set_root(Node* value) {
	root = value;
}

Node* RollbackSynchronizer::get_root() {
	return root;
}

void RollbackSynchronizer::set_enable_prediction(bool value) {
	enable_prediction = value;
}

bool RollbackSynchronizer::get_enable_prediction() {
	return enable_prediction;
}

void RollbackSynchronizer::set_state_properties(Array value) {
	state_properties = value;
}

Array RollbackSynchronizer::get_state_properties() {
	return state_properties;
}

void RollbackSynchronizer::set_full_state_interval(int value) {
	full_state_interval = value;
}

int RollbackSynchronizer::get_full_state_interval() {
	return full_state_interval;
}

void RollbackSynchronizer::set_diff_ack_interval(int value) {
	diff_ack_interval = value;
}

int RollbackSynchronizer::get_diff_ack_interval() {
	return diff_ack_interval;
}

void RollbackSynchronizer::set_input_properties(Array value) {
	input_properties = value;
}

Array RollbackSynchronizer::get_input_properties() {
	return input_properties;
}

void RollbackSynchronizer::set_enable_input_broadcast(bool value) {
	enable_input_broadcast = value;
}

bool RollbackSynchronizer::get_enable_input_broadcast() {
	return enable_input_broadcast;
}

void RollbackSynchronizer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("process_settings"), &RollbackSynchronizer::process_settings);
	ClassDB::bind_method(D_METHOD("process_authority"), &RollbackSynchronizer::process_authority);
	ClassDB::bind_method(D_METHOD("add_state", "node", "property"), &RollbackSynchronizer::add_state);
	ClassDB::bind_method(D_METHOD("add_input", "node", "property"), &RollbackSynchronizer::add_input);
	ClassDB::bind_method(D_METHOD("has_input"), &RollbackSynchronizer::has_input);
	ClassDB::bind_method(D_METHOD("get_input_age"), &RollbackSynchronizer::get_input_age);
	ClassDB::bind_method(D_METHOD("is_predicting"), &RollbackSynchronizer::is_predicting);
	ClassDB::bind_method(D_METHOD("ignore_prediction", "node"), &RollbackSynchronizer::ignore_prediction);
	ClassDB::bind_method(D_METHOD("get_last_known_input"), &RollbackSynchronizer::get_last_known_input);
	ClassDB::bind_method(D_METHOD("get_last_known_state"), &RollbackSynchronizer::get_last_known_state);
	ClassDB::bind_method(D_METHOD("set_root", "value"), &RollbackSynchronizer::set_root);
	ClassDB::bind_method(D_METHOD("get_root"), &RollbackSynchronizer::get_root);
	ClassDB::bind_method(D_METHOD("set_enable_prediction", "value"), &RollbackSynchronizer::set_enable_prediction);
	ClassDB::bind_method(D_METHOD("get_enable_prediction"), &RollbackSynchronizer::get_enable_prediction);
	ClassDB::bind_method(D_METHOD("set_state_properties", "value"), &RollbackSynchronizer::set_state_properties);
	ClassDB::bind_method(D_METHOD("get_state_properties"), &RollbackSynchronizer::get_state_properties);
	ClassDB::bind_method(D_METHOD("set_full_state_interval", "value"), &RollbackSynchronizer::set_full_state_interval);
	ClassDB::bind_method(D_METHOD("get_full_state_interval"), &RollbackSynchronizer::get_full_state_interval);
	ClassDB::bind_method(D_METHOD("set_diff_ack_interval", "value"), &RollbackSynchronizer::set_diff_ack_interval);
	ClassDB::bind_method(D_METHOD("get_diff_ack_interval"), &RollbackSynchronizer::get_diff_ack_interval);
	ClassDB::bind_method(D_METHOD("set_input_properties", "value"), &RollbackSynchronizer::set_input_properties);
	ClassDB::bind_method(D_METHOD("get_input_properties"), &RollbackSynchronizer::get_input_properties);
	ClassDB::bind_method(D_METHOD("set_enable_input_broadcast", "value"), &RollbackSynchronizer::set_enable_input_broadcast);
	ClassDB::bind_method(D_METHOD("get_enable_input_broadcast"), &RollbackSynchronizer::get_enable_input_broadcast);

	ClassDB::add_property(get_class_static(), PropertyInfo(Variant::OBJECT, "root"), "set_root", "get_root");
	ClassDB::add_property(get_class_static(), PropertyInfo(Variant::BOOL, "enable_prediction"), "set_enable_prediction", "get_enable_prediction");
	ClassDB::add_property_group(get_class_static(), "State","");
	ClassDB::add_property(get_class_static(), PropertyInfo(Variant::ARRAY, "state_properties"), "set_state_properties", "get_state_properties");
	ClassDB::add_property(get_class_static(), PropertyInfo(Variant::INT, "full_state_interval", PROPERTY_HINT_RANGE, "0,128,1,or_greater"), "set_full_state_interval", "get_full_state_interval");
	ClassDB::add_property(get_class_static(), PropertyInfo(Variant::INT, "diff_ack_interval", PROPERTY_HINT_RANGE, "0,128,1,or_greater"), "set_diff_ack_interval", "get_diff_ack_interval");
	ClassDB::add_property_group(get_class_static(), "Inputs","");
	ClassDB::add_property(get_class_static(), PropertyInfo(Variant::ARRAY, "input_properties"), "set_input_properties", "get_input_properties");
	ClassDB::add_property(get_class_static(), PropertyInfo(Variant::BOOL, "enable_input_broadcast"), "set_enable_input_broadcast", "get_enable_input_broadcast");
}

