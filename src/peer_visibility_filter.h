#pragma once

#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/classes/node.hpp>

using namespace godot;

class PeerVisibilityFilter : public Node {
	GDCLASS(PeerVisibilityFilter, Node);
public:
	enum UpdateMode {UPDATE_NEVER, UPDATE_ON_PEER, UPDATE_PER_TICK_LOOP, UPDATE_PER_TICK, UPDATE_PER_ROLLBACK_TICK};

protected:
	bool default_visibility = true;

	TypedArray<Callable> _visibility_filters;
	Dictionary _visibility_overrides;
	UpdateMode _update_mode = UpdateMode::UPDATE_ON_PEER;

	TypedArray<int> _visible_peers;
	TypedArray<int> _rpc_target_peers;

	static void _bind_methods();

public:
	PeerVisibilityFilter() = default;
	~PeerVisibilityFilter() override = default;

	void add_visibility_filter(Callable filter);

	void remove_visibility_filter(Callable filter);
	void clear_visibility_filters();

	bool get_visibility_for(int peer);
	void set_visibility_for(int peer, bool visibility);

	void unset_visibility_for(int peer);
	void update_visibility(PackedInt32Array peers = PackedInt32Array());
	TypedArray<int> get_visible_peers();
	TypedArray<int> get_rpc_target_peers();
	void set_update_mode(UpdateMode mode);
	UpdateMode get_update_mode();

	void _enter_tree() override;
	void _exit_tree() override;
	void _disconnect_update_handlers(UpdateMode mode);
	void _connect_update_handlers(UpdateMode mode);
	void _handle_peer_connect(Variant __);
	void _handle_peer_disconnect(Variant __);
	void _handle_tick(Variant _dt, Variant _t);
	void _handle_rollback_tick(Variant __);
};

VARIANT_ENUM_CAST(PeerVisibilityFilter::UpdateMode);