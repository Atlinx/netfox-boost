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
	static bool is_instance_valid(const Variant &p_instance);

	template <typename T>
	static bool is_instance_valid(Ref<T> ref)
	{
		return ref.is_valid();
	}

	static String vformatv(const String &p_text, const Array array);
};

#endif // UTILS_H