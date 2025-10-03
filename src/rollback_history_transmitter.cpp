
#include "rollback_history_transmitter.h"
#include "utils.h"

#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/multiplayer_api.hpp>
#include <godot_cpp/classes/multiplayer_peer.hpp>

Ref<_NetfoxLogger> _RollbackHistoryTransmitter::_logger;

void _RollbackHistoryTransmitter::_static_init()
{
	_logger = _NetfoxLogger::for_netfox("_RollbackHistoryTransmitter");
}

int _RollbackHistoryTransmitter::get_earliest_input_tick()
{
	return _earliest_input_tick;
}

int _RollbackHistoryTransmitter::get_latest_state_tick()
{
	return _latest_state_tick;
}

void _RollbackHistoryTransmitter::set_predicted_tick(Variant p_is_predicted_tick)
{
	_is_predicted_tick = p_is_predicted_tick;
}

void _RollbackHistoryTransmitter::sync_settings(Node* p_root, bool p_enable_input_broadcast, int p_full_state_interval, int p_diff_ack_interval)
{
	root = p_root;
	enable_input_broadcast = p_enable_input_broadcast;
	full_state_interval = p_full_state_interval;
	diff_ack_interval = p_diff_ack_interval;
}

void _RollbackHistoryTransmitter::configure(Ref<_PropertyHistoryBuffer> p_state_history, Ref<_PropertyHistoryBuffer> p_input_history, Ref<_PropertyConfig> p_state_property_config, Ref<_PropertyConfig> p_input_property_config, PeerVisibilityFilter* p_visibility_filter, Ref<PropertyCache> p_property_cache, HashSet<Node*> p_skipset)
{
	_state_history = p_state_history;
	_input_history = p_input_history;
	_state_property_config = p_state_property_config;
	_input_property_config = p_input_property_config;
	_visibility_filter = p_visibility_filter;
	_property_cache = p_property_cache;
	_skipset = p_skipset;

	_input_encoder = _RedundantHistoryEncoder::_new(_input_history, _property_cache);
	_full_state_encoder = _SnapshotHistoryEncoder::_new(_state_history, _property_cache);
	_diff_state_encoder = _DiffHistoryEncoder::_new(_state_history, _property_cache);

	_is_initialized = true;

	reset();
}

void _RollbackHistoryTransmitter::reset()
{
	_ackd_state.clear();
	int network_time_tick = Utils::get_autoload("NetworkTime")->get("tick");
	_latest_state_tick = network_time_tick - 1;
	_earliest_input_tick = network_time_tick;
	_next_full_state_tick = network_time_tick;
	_next_diff_ack_tick = network_time_tick;

	// Scatter full state sends, so not all nodes send at the same tick
	if(is_inside_tree())
	{
		_next_full_state_tick += UtilityFunctions::hash(root->get_path()) % Math::max(1, full_state_interval);
		_next_diff_ack_tick += UtilityFunctions::hash(root->get_path()) % Math::max(1, diff_ack_interval);
	}
	else
	{
		_next_full_state_tick += UtilityFunctions::hash(root->get_name()) % Math::max(1, full_state_interval);
		_next_diff_ack_tick += UtilityFunctions::hash(root->get_name()) % Math::max(1, diff_ack_interval);
	}

	_diff_state_encoder->add_properties(_state_property_config->get_properties());
	_full_state_encoder->set_properties(_get_owned_state_props());
	_input_encoder->set_properties(_get_owned_input_props());
}

void _RollbackHistoryTransmitter::conclude_tick_loop()
{
	int network_time_tick = Utils::get_autoload("NetworkTime")->get("tick");
	_earliest_input_tick = network_time_tick;
}

void _RollbackHistoryTransmitter::transmit_input(int tick)
{
	if(!_get_owned_input_props().is_empty())
	{
		int network_rollback_input_delay = Utils::get_autoload("NetworkRollback")->get("input_delay");
		int input_tick = tick + network_rollback_input_delay;
		Variant input_data = _input_encoder->encode(input_tick, _get_owned_input_props());
		int state_owning_peer = root->get_multiplayer_authority();

		if(enable_input_broadcast)
		{
			Array rpc_target_peers = _visibility_filter->get_rpc_target_peers();
			for(int i = 0; i < rpc_target_peers.size(); ++i)
			{
				int peer = rpc_target_peers[i];
				rpc_id(peer, "_submit_input", input_tick, input_data);
			}
		}
		else if(state_owning_peer != get_multiplayer()->get_unique_id())
		{
			rpc_id(state_owning_peer, "_submit_input", input_tick, input_data);
		}
	}
}

void _RollbackHistoryTransmitter::transmit_state(int tick)
{
	if(_get_owned_state_props().is_empty())
	{
		// We don't own state, don't transmit anything
		return ;
	}

	if(_is_predicted_tick && !_input_property_config->get_properties().is_empty())
	{
		// Don't transmit anything if we're predicting
		// EXCEPT when we're running inputless
		return ;
	}

	// Include properties we own
	Ref<_PropertySnapshot> full_state = _PropertySnapshot::_new();

	Array props = _get_owned_state_props();
	for(int i = 0; i < props.size(); ++i)
	{
		Ref<PropertyEntry> property = props[i];
		if(_should_broadcast(property, tick))
		{
			full_state->set_value(property->to_string(), property->get_value());
		}
	}

	emit_signal("_on_transmit_state", full_state, tick);

	// No properties to send?
	if(full_state->is_empty())
	{
		return ;
	}

	_latest_state_tick = Math::max(_latest_state_tick, tick);
	_state_history->merge(full_state, tick);

	bool is_sending_diffs = Utils::get_autoload("NetworkRollback")->get("enable_diff_states");
	bool is_full_state_tick = !is_sending_diffs || (full_state_interval > 0 && tick > _next_full_state_tick);

	if(is_full_state_tick)
	{

		// Broadcast new full state
		TypedArray<int> peers = _visibility_filter->get_rpc_target_peers();
		for(int i = 0; i < peers.size(); ++i)
		{
			int peer = peers[i];
			_send_full_state(tick, peer);
		}

		// Adjust next full state if sending diffs
		if(is_sending_diffs)
		{
			_next_full_state_tick = tick + full_state_interval;
		}
	}
	else
	{

		// Send diffs to each peer
		TypedArray<int> peers = _visibility_filter->get_visible_peers();
		for(int i = 0; i < peers.size(); ++i)
		{
			int peer = peers[i];
			int reference_tick = _ackd_state.get(peer, -1);
			if(reference_tick < 0 || !_state_history->has(reference_tick))
			{
				// Peer hasn't ack'd any tick, or we don't have the ack'd tick
				// Send full state
				_send_full_state(tick, peer);
				continue;
			}

			// Prepare diff
			Variant diff_state_data = _diff_state_encoder->encode(tick, reference_tick, _get_owned_state_props());

			if(_diff_state_encoder->get_full_snapshot().size() == _diff_state_encoder->get_encoded_snapshot().size())
			{
				// State is completely different, send full state
				_send_full_state(tick, peer);
			}
			else
			{
				// Send only diff
				rpc_id(peer, "_submit_diff_state", diff_state_data, tick, reference_tick);

				// Push metrics
				auto network_performance = Utils::get_autoload("NetworkPerformance");
				network_performance->call("push_full_state", _diff_state_encoder->get_full_snapshot());
				network_performance->call("push_sent_state", _diff_state_encoder->get_encoded_snapshot());
			}
		}
	}
}

bool _RollbackHistoryTransmitter::_should_broadcast(Ref<PropertyEntry> property, int tick)
{
	auto network_rollback = Utils::get_autoload("NetworkRollback");
	// Only broadcast if we've simulated the node
	// NOTE: _can_simulate checks mutations, but to override _skipset
	// we check first
	if(network_rollback->call("is_mutated", property->node, tick - 1))
		return true;
	if(_skipset.has(property->node))
		return false;
	if(network_rollback->call("is_rollback_aware", property->node))
		return network_rollback->call("is_simulated", property->node);

	// Node is not rollback-aware, broadcast updates only if we own it
	return property->node->is_multiplayer_authority();
}

void _RollbackHistoryTransmitter::_send_full_state(int tick, int peer)
{
	Dictionary full_state_snapshot = _state_history->get_snapshot(tick)->as_dictionary();
	Variant full_state_data = _full_state_encoder->encode(tick, _get_owned_state_props());

	auto network_performance = Utils::get_autoload("NetworkPerformance");

	rpc_id(peer, "_submit_full_state", full_state_data, tick);

	if(peer <= 0)
	{
		network_performance->call("push_full_state_broadcast", full_state_snapshot);
		network_performance->call("push_sent_state_broadcast", full_state_snapshot);
	}
	else
	{
		network_performance->call("push_full_state", full_state_snapshot);
		network_performance->call("push_sent_state", full_state_snapshot);
	}
}

void _RollbackHistoryTransmitter::_submit_input(int tick, Array data)
{
	if(!_is_initialized)
	{
		// Settings not processed yet
		return;
	}

	int sender = get_multiplayer()->get_remote_sender_id();
	TypedArray<_PropertySnapshot> snapshots = _input_encoder->decode(data, _input_property_config->get_properties_owned_by(sender));
	int earliest_received_input = _input_encoder->apply(tick, snapshots, sender);
	if(earliest_received_input >= 0)
		_earliest_input_tick = Math::min(_earliest_input_tick, earliest_received_input);
}

void _RollbackHistoryTransmitter::_submit_full_state(Array data, int tick)
{
	if(!_is_initialized)
	{
		// Settings not processed yet
		return ;
	}

	int sender = get_multiplayer()->get_remote_sender_id();
	Variant snapshot = _full_state_encoder->decode(data, _state_property_config->get_properties_owned_by(sender));
	if(!_full_state_encoder->apply(tick, snapshot, sender))
	{
		// Invalid data
		return ;
	}

	bool network_rollback_enable_diff_states = Utils::get_autoload("NetworkRollback")->get("enable_diff_states");

	_latest_state_tick = tick;
	if(network_rollback_enable_diff_states)
		rpc_id(sender, "_ack_full_state", tick);
}

void _RollbackHistoryTransmitter::_submit_diff_state(PackedByteArray data, int tick, int reference_tick)
{
	if(!_is_initialized)
	{
		// Settings not processed yet
		return ;
	}

	int sender = get_multiplayer()->get_remote_sender_id();
	Variant diff_snapshot = _diff_state_encoder->decode(data, _state_property_config->get_properties_owned_by(sender));
	if(!_diff_state_encoder->apply(tick, diff_snapshot, reference_tick, sender))
	{
		// Invalid data
		return ;
	}

	_latest_state_tick = tick;

	bool network_rollback_enable_diff_states = Utils::get_autoload("NetworkRollback")->get("enable_diff_states");
	if(network_rollback_enable_diff_states)
	{
		if(diff_ack_interval > 0 && tick > _next_diff_ack_tick)
		{
			rpc_id(sender, "_ack_diff_state", tick);
			_next_diff_ack_tick = tick + diff_ack_interval;
		}
	}
}

void _RollbackHistoryTransmitter::_ack_full_state(int tick)
{
	int sender_id = get_multiplayer()->get_remote_sender_id();
	_ackd_state[sender_id] = tick;

	_logger->trace(vformat("Peer %d ack'd full state for tick %d", sender_id, tick));
}

void _RollbackHistoryTransmitter::_ack_diff_state(int tick)
{
	int sender_id = get_multiplayer()->get_remote_sender_id();
	_ackd_state[sender_id] = tick;

	_logger->trace(vformat("Peer %d ack'd diff state for tick %d", sender_id, tick));
}

TypedArray<PropertyEntry> _RollbackHistoryTransmitter::_get_recorded_state_props()
{
	return _state_property_config->get_properties();
}

TypedArray<PropertyEntry> _RollbackHistoryTransmitter::_get_owned_state_props()
{
	return _state_property_config->get_owned_properties();
}

TypedArray<PropertyEntry> _RollbackHistoryTransmitter::_get_recorded_input_props()
{
	return _input_property_config->get_owned_properties();
}

TypedArray<PropertyEntry> _RollbackHistoryTransmitter::_get_owned_input_props()
{
	return _input_property_config->get_owned_properties();
}

void _RollbackHistoryTransmitter::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_earliest_input_tick"), &_RollbackHistoryTransmitter::get_earliest_input_tick);
	ClassDB::bind_method(D_METHOD("get_latest_state_tick"), &_RollbackHistoryTransmitter::get_latest_state_tick);
	ClassDB::bind_method(D_METHOD("set_predicted_tick", "p_is_predicted_tick"), &_RollbackHistoryTransmitter::set_predicted_tick);
	ClassDB::bind_method(D_METHOD("sync_settings", "p_root", "p_enable_input_broadcast", "p_full_state_interval", "p_diff_ack_interval"), &_RollbackHistoryTransmitter::sync_settings);
	ClassDB::bind_method(D_METHOD("reset"), &_RollbackHistoryTransmitter::reset);
	ClassDB::bind_method(D_METHOD("conclude_tick_loop"), &_RollbackHistoryTransmitter::conclude_tick_loop);
	ClassDB::bind_method(D_METHOD("transmit_input", "tick"), &_RollbackHistoryTransmitter::transmit_input);
	ClassDB::bind_method(D_METHOD("transmit_state", "tick"), &_RollbackHistoryTransmitter::transmit_state);

	ClassDB::add_signal(get_class_static(), MethodInfo("_on_transmit_state", PropertyInfo(Variant::DICTIONARY, "state"), PropertyInfo(Variant::INT, "tick")));

	ClassDB::bind_method(D_METHOD("_submit_input", "tick", "data"), &_RollbackHistoryTransmitter::_submit_input);
	ClassDB::bind_method(D_METHOD("_submit_full_state", "data", "tick"), &_RollbackHistoryTransmitter::_submit_full_state);
	ClassDB::bind_method(D_METHOD("_submit_diff_state", "data", "tick", "reference_tick"), &_RollbackHistoryTransmitter::_submit_diff_state);
	ClassDB::bind_method(D_METHOD("_ack_full_state", "tick"), &_RollbackHistoryTransmitter::_ack_full_state);
	ClassDB::bind_method(D_METHOD("_ack_diff_state", "tick"), &_RollbackHistoryTransmitter::_ack_diff_state);
}

