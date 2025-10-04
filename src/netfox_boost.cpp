#include "netfox_boost.h"

#include <godot_cpp/classes/multiplayer_api.hpp>
#include <godot_cpp/classes/multiplayer_peer.hpp>

void NetfoxBoost::_bind_methods() {
	godot::ClassDB::bind_method(D_METHOD("print_type", "variant"), &NetfoxBoost::print_type);
	godot::ClassDB::bind_method(D_METHOD("test_rpc", "msg"), &NetfoxBoost::test_rpc);
	godot::ClassDB::bind_method(D_METHOD("rpc_receive", "msg"), &NetfoxBoost::rpc_receive);
}

NetfoxBoost::NetfoxBoost() {
	Dictionary dict;
	dict["rpc_mode"] = MultiplayerAPI::RPC_MODE_ANY_PEER;
	dict["transfer_mode"] = MultiplayerPeer::TRANSFER_MODE_RELIABLE;
	dict["call_local"] = true;
	rpc_config("rpc_receive", dict);
}

void NetfoxBoost::print_type(Variant p_variant) {
	print_line(vformat("Type: %d", p_variant.get_type()));
}

void NetfoxBoost::test_rpc(String p_msg) {
	print_line(vformat("Sending RPC with msg: %s", p_msg));

	rpc("rpc_receive", p_msg);
}

void NetfoxBoost::rpc_receive(String p_msg) {
	print_line(vformat("Received RPC with msg: %s", p_msg));
}

