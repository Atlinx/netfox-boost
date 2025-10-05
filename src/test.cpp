#include "test.h"

#include "utils.h"

#include <godot_cpp/classes/multiplayer_api.hpp>
#include <godot_cpp/classes/multiplayer_peer.hpp>

CPPTest::CPPTest() 
{
	Dictionary dict;
	dict["rpc_mode"] = MultiplayerAPI::RPC_MODE_ANY_PEER;
	dict["transfer_mode"] = MultiplayerPeer::TRANSFER_MODE_RELIABLE;
	dict["call_local"] = true;
	rpc_config("rpc_receive", dict);
}

void CPPTest::print_type(Variant p_variant) 
{
	print_line(vformat("Type: %d", p_variant.get_type()));
}

void CPPTest::test_rpc(String p_msg) 
{
	print_line(vformat("Sending RPC with msg: %s", p_msg));

	rpc("rpc_receive", p_msg);
}

void CPPTest::rpc_receive(String p_msg) 
{
	print_line(vformat("Received RPC with msg: %s", p_msg));
}

void CPPTest::set_my_string(String value)
{
	my_string = value;
}

String CPPTest::get_my_string()
{
	return my_string;
}

Node* CPPTest::test_get_autoload(String autoload)
{
	return Utils::get_autoload(autoload);
}

Variant CPPTest::test_get_autoload_value(String autoload, String field)
{
	return Utils::get_autoload(autoload)->get(field);
}

void CPPTest::_bind_methods() 
{
	godot::ClassDB::bind_method(D_METHOD("print_type", "variant"), &CPPTest::print_type);
	godot::ClassDB::bind_method(D_METHOD("test_rpc", "msg"), &CPPTest::test_rpc);
	godot::ClassDB::bind_method(D_METHOD("rpc_receive", "msg"), &CPPTest::rpc_receive);
	godot::ClassDB::bind_method(D_METHOD("test_get_autoload", "autoload"), &CPPTest::test_get_autoload);
	godot::ClassDB::bind_method(D_METHOD("test_get_autoload_value", "autoload", "field"), &CPPTest::test_get_autoload_value);

	godot::ClassDB::bind_method(D_METHOD("set_my_string", "value"), &CPPTest::set_my_string);
	godot::ClassDB::bind_method(D_METHOD("get_my_string"), &CPPTest::get_my_string);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "my_string"), "set_my_string", "get_my_string");
	ADD_SIGNAL(MethodInfo("my_signal_no_args")); 
}


