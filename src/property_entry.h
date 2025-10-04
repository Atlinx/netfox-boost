#pragma once

#include "logger.h"

#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

using namespace godot;

class PropertyEntry : public RefCounted {
	GDCLASS(PropertyEntry, RefCounted);
public:

protected:
	static Ref<_NetfoxLogger> _logger;
	static void _bind_methods();

public:
	PropertyEntry() = default;
	~PropertyEntry() override = default;

	static Ref<PropertyEntry> parse(Node* root, String path);
	static String make_path(Node* root, Variant node, String property);

	String path;
	Node* node;
	String property;

	Variant get_value();
	void set_value(Variant value);
	bool is_valid();
	String to_string() const;
};