#include "utils.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/variant/node_path.hpp>

using namespace godot;

Node* Utils::get_autoload(String autoload) {
	Engine *engine = Engine::get_singleton();
	SceneTree *scene_tree = Object::cast_to<SceneTree>(engine->get_main_loop());
	
	if (!scene_tree)
		return nullptr;

	Window *root = scene_tree->get_root();
	Node *autoload_node = root->get_node<Node>(NodePath(autoload));

	if (autoload_node)
		return autoload_node; 
	return nullptr;
}

bool Utils::is_instance_valid(const Variant &p_instance)
{
	if (p_instance.get_type() != Variant::OBJECT) {
		return false;
	}
	return p_instance.get_validated_object() != nullptr;
}