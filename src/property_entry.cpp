
#include "property_entry.h"
#include "utils.h"

#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

Ref<_NetfoxLogger> PropertyEntry::_logger;

Variant PropertyEntry::get_value()
{
	return node->get_indexed(property);
}

void PropertyEntry::set_value(Variant value)
{
	node->set_indexed(property, value);
}

bool PropertyEntry::is_valid()
{
	if (!Utils::is_instance_valid(node))
		return false;

	// Return true if node has given property
	auto property_list = node->get_property_list();
	for (int i = 0; i < property_list.size(); ++i) {
		Dictionary dict = property_list[i];
		if (dict["name"] == property)
			return true;
	}
	return false;
}

String PropertyEntry::to_string() const
{
	return path;
}

Ref<PropertyEntry> PropertyEntry::parse(Node* root, String path)
{
	if (_logger.is_null())
		_logger = _NetfoxLogger::for_netfox("PropertyEntry");

		Ref<PropertyEntry> result;
	result.instantiate();
	result->node = root->get_node<Node>(NodePath(path));
	result->property = path.erase(0, path.find(":") + 1);
	result->path = path;
	return result;
}

String PropertyEntry::make_path(Node* root, Variant node, String property)
{
	String node_path = "";

	Variant::Type type = node.get_type();
	if(type == Variant::STRING)
	{
		node_path = node;
	}
	else if(type == Variant::NODE_PATH)
	{
		node_path = String(node);
	}
	else if(Object::cast_to<Node>(node))
	{
		Node* _node = Object::cast_to<Node>(node);
		node_path = String(root->get_path_to(_node));
	}
	else
	{
		_logger->error(vformat("Can't stringify node reference: %s", node));
		return "";
	}

	if(node_path == ".")
	{
		node_path = "";
	}

	return vformat("%s:%s", node_path, property);
}

void PropertyEntry::_bind_methods() 
{
	_logger = _NetfoxLogger::for_netfox("PropertyEntry");
	
	ClassDB::bind_static_method("PropertyEntry", D_METHOD("parse", "root", "path"), &PropertyEntry::parse);
	ClassDB::bind_static_method("PropertyEntry", D_METHOD("make_path", "root", "node", "property"), &PropertyEntry::make_path);
	ClassDB::bind_method(D_METHOD("get_value"), &PropertyEntry::get_value);
	ClassDB::bind_method(D_METHOD("set_value", "value"), &PropertyEntry::set_value);
	ClassDB::bind_method(D_METHOD("is_valid"), &PropertyEntry::is_valid);
}

