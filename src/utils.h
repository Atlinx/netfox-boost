#ifndef UTILS_H
#define UTILS_H

#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/window.hpp>

using namespace godot;

class Utils : public RefCounted {
	GDCLASS(Utils, RefCounted);

public:
	static Node* get_autoload(String autoload);
	static bool is_instance_valid(Variant variant);
};

#endif // UTILS_H