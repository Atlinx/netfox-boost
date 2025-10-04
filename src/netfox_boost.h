#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/wrapped.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/classes/node.hpp>

using namespace godot;

class NetfoxBoost : public Node {
	GDCLASS(NetfoxBoost, Node)

protected:
	static void _bind_methods();

	String my_string;

public:
	NetfoxBoost();
	~NetfoxBoost() override = default;

	void print_type(Variant p_variant);
	void test_rpc(String msg);
	void rpc_receive(String msg);

	void set_my_string(String);
	String get_my_string();
};
