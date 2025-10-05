#include "peer_visibility_filter.h"
#include "utils.h"

#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/multiplayer_api.hpp>
#include <godot_cpp/classes/multiplayer_peer.hpp>

void PeerVisibilityFilter::add_visibility_filter(Callable filter)
{
  if(!_visibility_filters.has(filter))
    _visibility_filters.append(filter);
}

void PeerVisibilityFilter::remove_visibility_filter(Callable filter)
{
  _visibility_filters.erase(filter);
}

void PeerVisibilityFilter::clear_visibility_filters()
{
  _visibility_filters.clear();
}

bool PeerVisibilityFilter::get_visibility_for(int peer)
{
  for(int i = 0; i < _visibility_filters.size(); ++i)
  {
    Callable filter = _visibility_filters[i];
    if(!filter.call(peer))
      return false;
  }
  return _visibility_overrides.get(peer, default_visibility);
}

void PeerVisibilityFilter::set_visibility_for(int peer, bool visibility)
{
  if(peer == 0)
    default_visibility = visibility;
  else
    _visibility_overrides[peer] = visibility;
}

void PeerVisibilityFilter::unset_visibility_for(int peer)
{
  _visibility_overrides.erase(peer);
}

void PeerVisibilityFilter::update_visibility(PackedInt32Array peers)
{
  if (peers.size() == 0) 
    peers = get_multiplayer()->get_peers();

  // Find visible peers
  _visible_peers.clear();
  for(int i = 0; i < peers.size(); ++i)
  {
    int peer = peers[i];
    if(get_visibility_for(peer))
    {
      _visible_peers.append(peer);
    }
  }

  // Decide how many RPC calls are needed to cover visible peers
  if(_visible_peers.size() == peers.size())
  {

    // Everyone is visible -> broadcast
    _rpc_target_peers.clear();
    _rpc_target_peers.append(MultiplayerPeer::TARGET_PEER_BROADCAST);
  }
  else if(_visible_peers.size() == peers.size() - 1)
  {

    // Only a single peer is missing, exclude that
    for(int i = 0; i < peers.size(); ++i)
    {
      int peer = peers[i];
      if(!_visible_peers.has(peer))
      {
        _rpc_target_peers.clear();
        _rpc_target_peers.append(-peer);
        break;
      }
    }
  }
  else
  {
    // Custom list, can't optimize RPC call count
    _rpc_target_peers = _visible_peers;

    // Don't include self in RPC target list
    if(get_multiplayer().is_valid())
      _rpc_target_peers.erase(get_multiplayer()->get_unique_id());
  }
}

TypedArray<int> PeerVisibilityFilter::get_visible_peers()
{
  return _visible_peers;
}

TypedArray<int> PeerVisibilityFilter::get_rpc_target_peers()
{
  return _rpc_target_peers;
}

void PeerVisibilityFilter::set_update_mode(PeerVisibilityFilter::UpdateMode mode)
{
  _disconnect_update_handlers(_update_mode);
  _connect_update_handlers(mode);
  _update_mode = mode;
}

PeerVisibilityFilter::UpdateMode PeerVisibilityFilter::get_update_mode()
{
  return _update_mode;
}

void PeerVisibilityFilter::_enter_tree()
{
	if (!Engine::get_singleton() || Engine::get_singleton()->is_editor_hint())
		return;
  _connect_update_handlers(_update_mode);
  if(get_multiplayer().is_valid())
    update_visibility();
}

void PeerVisibilityFilter::_exit_tree()
{
	if (!Engine::get_singleton() || Engine::get_singleton()->is_editor_hint())
		return;
  _disconnect_update_handlers(_update_mode);
}

void PeerVisibilityFilter::_disconnect_update_handlers(PeerVisibilityFilter::UpdateMode mode)
{
  switch (mode) {
    case UpdateMode::UPDATE_NEVER:
      break;

    case UpdateMode::UPDATE_ON_PEER: {
      Ref<MultiplayerAPI> mp_api = get_multiplayer();
      
      // Use disconnect instead of connect
      mp_api->disconnect(StringName("peer_connected"), Callable(this, StringName("_handle_peer_connect")));
      mp_api->disconnect(StringName("peer_disconnected"), Callable(this, StringName("_handle_peer_disconnect")));
    } break;

    case UpdateMode::UPDATE_PER_TICK_LOOP: {
      // Disconnect from NetworkTime.before_tick_loop
      Node *network_time_node = Utils::get_autoload("NetworkTime");
      if (network_time_node) {
        network_time_node->disconnect(StringName("before_tick_loop"), Callable(this, StringName("update_visibility")));
      }
    } break;

    case UpdateMode::UPDATE_PER_TICK: {
      // Disconnect from NetworkTime.before_tick
      Node *network_time_node = Utils::get_autoload("NetworkTime");
      if (network_time_node) {
        network_time_node->disconnect(StringName("before_tick"), Callable(this, StringName("_handle_tick")));
      }
    } break;

    case UpdateMode::UPDATE_PER_ROLLBACK_TICK: {
      // Disconnect from NetworkRollback.after_process_tick
      Node *network_rollback_node = Utils::get_autoload("NetworkRollback");
      if (network_rollback_node) {
        network_rollback_node->disconnect(StringName("after_process_tick"), Callable(this, StringName("_handle_rollback_tick")));
      }
    } break;

    default: {
      // Assertion equivalent
      String error_message = "Unhandled update mode! ";
      error_message += String::num_int64((int)mode);
      ERR_FAIL_MSG(error_message);
    } break;
  }
}

void PeerVisibilityFilter::_connect_update_handlers(PeerVisibilityFilter::UpdateMode mode)
{
  switch (mode) {
    case UpdateMode::UPDATE_NEVER:
      break;
    
    case UpdateMode::UPDATE_ON_PEER: {
      Ref<MultiplayerAPI> mp_api = get_multiplayer();
      
      // Connect to the Multiplayer API signals
      mp_api->connect(StringName("peer_connected"), Callable(this, StringName("_handle_peer_connect")));
      mp_api->connect(StringName("peer_disconnected"), Callable(this, StringName("_handle_peer_disconnect")));
    } break;

    case UpdateMode::UPDATE_PER_TICK_LOOP: {
      // Connect to NetworkTime.before_tick_loop
      Node *network_time_node = Utils::get_autoload("NetworkTime");
      if (network_time_node) {
        network_time_node->connect(StringName("before_tick_loop"), Callable(this, StringName("update_visibility")));
      }
    } break;

    case UpdateMode::UPDATE_PER_TICK: {
      // Connect to NetworkTime.before_tick
      Node *network_time_node = Utils::get_autoload("NetworkTime");
      if (network_time_node) {
        network_time_node->connect(StringName("before_tick"), Callable(this, StringName("_handle_tick")));
      }
    } break;

    case UpdateMode::UPDATE_PER_ROLLBACK_TICK: {
      // Connect to NetworkRollback.after_process_tick
      Node *network_rollback_node = Utils::get_autoload("NetworkRollback");
      if (network_rollback_node) {
        network_rollback_node->connect(StringName("after_process_tick"), Callable(this, StringName("_handle_rollback_tick")));
      }
    } break;

    default: {
      // Assertion equivalent
      String error_message = "Unhandled update mode! ";
      error_message += String::num_int64((int)mode);
      
      // Using Godot's built-in error handling
      ERR_FAIL_MSG(error_message);
    } break;
  }
}

void PeerVisibilityFilter::_handle_peer_connect(Variant __)
{
  update_visibility();
}

void PeerVisibilityFilter::_handle_peer_disconnect(Variant __)
{
  update_visibility();
}

void PeerVisibilityFilter::_handle_tick(Variant _dt, Variant _t)
{
  update_visibility();
}

void PeerVisibilityFilter::_handle_rollback_tick(Variant __)
{
  update_visibility();
}

void PeerVisibilityFilter::_bind_methods() {
  // UtilityFunctions::print cannot be used here as it's for binding methods, not runtime logic.
  ClassDB::bind_method(D_METHOD("add_visibility_filter", "filter"), &PeerVisibilityFilter::add_visibility_filter);
  ClassDB::bind_method(D_METHOD("remove_visibility_filter", "filter"), &PeerVisibilityFilter::remove_visibility_filter);
  ClassDB::bind_method(D_METHOD("clear_visibility_filters"), &PeerVisibilityFilter::clear_visibility_filters);
  ClassDB::bind_method(D_METHOD("get_visibility_for", "peer"), &PeerVisibilityFilter::get_visibility_for);
  ClassDB::bind_method(D_METHOD("set_visibility_for", "peer", "visibility"), &PeerVisibilityFilter::set_visibility_for);
  ClassDB::bind_method(D_METHOD("unset_visibility_for", "peer"), &PeerVisibilityFilter::unset_visibility_for);
  ClassDB::bind_method(D_METHOD("update_visibility", "peers"), &PeerVisibilityFilter::update_visibility);
  ClassDB::bind_method(D_METHOD("get_visible_peers"), &PeerVisibilityFilter::get_visible_peers);
  ClassDB::bind_method(D_METHOD("get_rpc_target_peers"), &PeerVisibilityFilter::get_rpc_target_peers);
  ClassDB::bind_method(D_METHOD("set_update_mode", "mode"), &PeerVisibilityFilter::set_update_mode);
  ClassDB::bind_method(D_METHOD("get_update_mode"), &PeerVisibilityFilter::get_update_mode);
  
  BIND_ENUM_CONSTANT(UPDATE_NEVER);
  BIND_ENUM_CONSTANT(UPDATE_ON_PEER);
  BIND_ENUM_CONSTANT(UPDATE_PER_TICK_LOOP);
  BIND_ENUM_CONSTANT(UPDATE_PER_TICK);
  BIND_ENUM_CONSTANT(UPDATE_PER_ROLLBACK_TICK);
}