#include "property_entry.h"
#include "utils.h"

#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

Ref<_NetfoxLogger> PropertyEntry::_logger;

// Getters and Setters implementation
String PropertyEntry::get_path() const
{
	return path;
}

void PropertyEntry::set_path(const String& p_path)
{
	path = p_path;
}

Node* PropertyEntry::get_node() const
{
	return node;
}

void PropertyEntry::set_node(Node* p_node)
{
	node = p_node;
}

String PropertyEntry::get_property() const
{
	return property;
}

void PropertyEntry::set_property(const String& p_property)
{
	property = p_property;
}

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

	for (Dictionary dict : node->get_property_list()) 
	{
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

	ClassDB::bind_method(D_METHOD("get_path"), &PropertyEntry::get_path);
	ClassDB::bind_method(D_METHOD("set_path", "p_path"), &PropertyEntry::set_path);

	ClassDB::bind_method(D_METHOD("get_node"), &PropertyEntry::get_node);
	ClassDB::bind_method(D_METHOD("set_node", "p_node"), &PropertyEntry::set_node);

	ClassDB::bind_method(D_METHOD("get_property"), &PropertyEntry::get_property);
	ClassDB::bind_method(D_METHOD("set_property", "p_property"), &PropertyEntry::set_property);

	ClassDB::bind_method(D_METHOD("get_value"), &PropertyEntry::get_value);
	ClassDB::bind_method(D_METHOD("set_value", "value"), &PropertyEntry::set_value);
	ClassDB::bind_method(D_METHOD("is_valid"), &PropertyEntry::is_valid);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "path"), "set_path", "get_path");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "node", PROPERTY_HINT_NODE_TYPE, "Node"), "set_node", "get_node");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "property"), "set_property", "get_property");
}