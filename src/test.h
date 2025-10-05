#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/wrapped.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/classes/node.hpp>

using namespace godot;

class CPPTest : public Node {
	GDCLASS(CPPTest, Node)

protected:
	static void _bind_methods();

	String my_string;

public:
	CPPTest();
	~CPPTest() override = default;

	void print_type(Variant p_variant);
	void test_rpc(String msg);
	void rpc_receive(String msg);
	Node* test_get_autoload(String autoload);
	Variant test_get_autoload_value(String autoload, String field);

	void set_my_string(String);
	String get_my_string();
};
