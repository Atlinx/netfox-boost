
#include "rollback_history_recorder.h"
#include "utils.h"

#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

void _RollbackHistoryRecorder::configure(Ref<_PropertyHistoryBuffer> p_state_history, Ref<_PropertyHistoryBuffer> p_input_history, Ref<_PropertyConfig> p_state_property_config, Ref<_PropertyConfig> p_input_property_config, Ref<PropertyCache> p_property_cache, HashSet<Node*> p_skipset)
{
	_state_history = p_state_history;
	_input_history = p_input_history;
	_state_property_config = p_state_property_config;
	_input_property_config = p_input_property_config;
	_property_cache = p_property_cache;
	_skipset = p_skipset;
}

void _RollbackHistoryRecorder::set_latest_state_tick(int p_latest_state_tick)
{
	_latest_state_tick = p_latest_state_tick;
}

void _RollbackHistoryRecorder::apply_state(int tick)
{

	// Apply state for tick
	Ref<_PropertySnapshot> state = _state_history->get_history(tick);
	state->apply(_property_cache);
}

void _RollbackHistoryRecorder::apply_display_state()
{
	apply_state(Utils::get_autoload("NetworkRollback")->get("display_tick"));
}

void _RollbackHistoryRecorder::apply_tick(int tick)
{
	Ref<_PropertySnapshot> state = _state_history->get_history(tick);
	Ref<_PropertySnapshot> input = _input_history->get_history(tick);

	state->apply(_property_cache);
	input->apply(_property_cache);
}

void _RollbackHistoryRecorder::trim_history()
{

	// Trim history
	_state_history->trim();
	_input_history->trim();
}

void _RollbackHistoryRecorder::record_input(int tick)
{

	// Record input
	if(!_get_recorded_input_props().is_empty())
	{
		Ref<_PropertySnapshot> input = _PropertySnapshot::extract(_get_recorded_input_props());
		int input_tick = tick + (int) Utils::get_autoload("NetworkRollback")->get("input_delay");
		_input_history->set_snapshot(input_tick, input);
	}
}

void _RollbackHistoryRecorder::record_state(int tick)
{
	// Record state for specified tick ( current + 1 )
	// Check if any of the managed nodes were mutated
    Node* network_rollback = Utils::get_autoload("NetworkRollback");
    if (!network_rollback) {
        ERR_FAIL_MSG("NetworkRollback autoload not found for mutation check.");
        return;
    }

    bool is_mutated = false;
    for (Ref<PropertyEntry> pe : _get_recorded_state_props()) {
        if (Object::cast_to<Node>(pe->node)) {
            is_mutated = network_rollback->call("is_mutated", pe->node, tick - 1);
            if (is_mutated)
                break;
        }
    }
    
    Ref<_PropertySnapshot> record_state = _PropertySnapshot::extract(_get_state_props_to_record(tick));
    if (record_state.is_valid() && record_state->size() > 0)
    {
        Ref<_PropertySnapshot> merge_state = _state_history->get_history(tick - 1);
        
        if (merge_state.is_valid())
            _state_history->set_snapshot(tick, merge_state->merge(record_state));
        else
            _state_history->set_snapshot(tick, record_state);
    }
}

bool _RollbackHistoryRecorder::_should_record_tick(int tick)
{
	TypedArray<PropertyEntry> recorded_state_props = _get_recorded_state_props();

	if (recorded_state_props.is_empty()) {
		// Don't record tick if there's no props to record
		return false;
	}

	Node* network_rollback = Utils::get_autoload("NetworkRollback");
	if (!network_rollback) {
		ERR_FAIL_V_MSG(false, "NetworkRollback autoload not found for mutation check.");
	}

	bool was_mutated = false;
	for (Ref<PropertyEntry> pe : recorded_state_props) {
		if (pe->node) {
			was_mutated = network_rollback->call("is_mutated", pe->node, tick - 1);
			if (was_mutated)
				break;
		}
	}
    
	if (was_mutated) {
		// If there's any node that was mutated, there's something to record
		return true;
	}

	// Otherwise, record only if we don't have authoritative state for the tick
	return tick > _latest_state_tick;
}

bool _RollbackHistoryRecorder::_should_record_property(Ref<PropertyEntry> property_entry, int tick)
{
	Node* network_rollback = Utils::get_autoload("NetworkRollback");
	if (!network_rollback) {
		ERR_FAIL_V_MSG(false, "NetworkRollback autoload not found for property mutation check.");
	}
	if (network_rollback->call("is_mutated", property_entry->node, tick - 1))
		return true;
	if (_skipset.has(property_entry->node))
		return false;
	return true;
}

Array _RollbackHistoryRecorder::_get_state_props_to_record(int tick)
{
	if (!_should_record_tick(tick)) {
		return Array(); // Equivalent of []
	}

	Array recorded_state_props = _get_recorded_state_props();

	// Assuming _skipset is a godot::HashSet (which has .is_empty())
	if (_skipset.is_empty()) {
		return recorded_state_props;
	}

	// Implementation of .filter(func(pe): return _should_record_property(pe, tick))
	Array filtered_props;
	for (Ref<PropertyEntry> pe : recorded_state_props) {
		if (_should_record_property(pe, tick)) {
			filtered_props.push_back(pe);
		}
	}

	return filtered_props;
}

TypedArray<PropertyEntry> _RollbackHistoryRecorder::_get_recorded_state_props()
{
	return _state_property_config->get_properties();
}

TypedArray<PropertyEntry> _RollbackHistoryRecorder::_get_owned_state_props()
{
	return _state_property_config->get_owned_properties();
}

TypedArray<PropertyEntry> _RollbackHistoryRecorder::_get_recorded_input_props()
{
	return _input_property_config->get_owned_properties();
}

TypedArray<PropertyEntry> _RollbackHistoryRecorder::_get_owned_input_props()
{
	return _input_property_config->get_owned_properties();
}

void _RollbackHistoryRecorder::_bind_methods() {
	// ClassDB::bind_method(D_METHOD("configure", "p_state_history", "p_input_history", "p_state_property_config", "p_input_property_config", "p_property_cache", "p_skipset"), &_RollbackHistoryRecorder::configure);
	ClassDB::bind_method(D_METHOD("set_latest_state_tick", "p_latest_state_tick"), &_RollbackHistoryRecorder::set_latest_state_tick);
	ClassDB::bind_method(D_METHOD("apply_state", "tick"), &_RollbackHistoryRecorder::apply_state);
	ClassDB::bind_method(D_METHOD("apply_display_state"), &_RollbackHistoryRecorder::apply_display_state);
	ClassDB::bind_method(D_METHOD("apply_tick", "tick"), &_RollbackHistoryRecorder::apply_tick);
	ClassDB::bind_method(D_METHOD("trim_history"), &_RollbackHistoryRecorder::trim_history);
	ClassDB::bind_method(D_METHOD("record_input", "tick"), &_RollbackHistoryRecorder::record_input);
	ClassDB::bind_method(D_METHOD("record_state", "tick"), &_RollbackHistoryRecorder::record_state);
}

