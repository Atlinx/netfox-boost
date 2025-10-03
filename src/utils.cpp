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

bool is_instance_valid(Variant variant)
{
	if (Object::cast_to<Node>(variant)) {
		Node* node = Object::cast_to<Node>(variant);
		return node != nullptr && UtilityFunctions::is_instance_id_valid(node->get_instance_id());
	} else {
		return variant.get_type()!= Variant::NIL;
	}
}