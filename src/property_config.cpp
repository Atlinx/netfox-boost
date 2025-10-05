#include "property_config.h"
#include "utils.h"

#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

void _PropertyConfig::set_local_peer_id(int p_id)
{
	local_peer_id = p_id;
	_auth_properties.clear();
}

int _PropertyConfig::get_local_peer_id() const
{
	return local_peer_id;
}

void _PropertyConfig::clear()
{
	_properties.clear();
	_auth_properties.clear();
}

void _PropertyConfig::set_properties(TypedArray<PropertyEntry> p_properties)
{
	clear();
	
	_properties.clear();
	_properties.resize(p_properties.size());
	for (int i = 0; i < p_properties.size(); ++i) {
		_properties.set(i, p_properties[i]);
	}
}

void _PropertyConfig::set_properties_from_paths(Array property_paths, Ref<PropertyCache> property_cache)
{
	clear();
	for(int i = 0; i < property_paths.size(); ++i)
	{
		String path = property_paths[i];
		_properties.push_back(property_cache->get_entry(path));
	}
}

TypedArray<PropertyEntry> _PropertyConfig::get_properties() const
{
	TypedArray<PropertyEntry> result;
	result.resize(_properties.size());
	for (int i = 0; i < _properties.size(); ++i) {
		result.set(i, _properties[i]);
	}
	return result;
}

TypedArray<PropertyEntry> _PropertyConfig::get_owned_properties()
{
	return get_properties_owned_by(local_peer_id);
}

TypedArray<PropertyEntry> _PropertyConfig::get_properties_owned_by(int peer)
{
	if(!_auth_properties.has(peer))
	{
		Vector<Ref<PropertyEntry>> owned_properties;
		
		for(int i = 0; i < _properties.size(); ++i)
		{
			Ref<PropertyEntry> property_entry = _properties[i];
			if(Utils::is_instance_valid(property_entry) && Utils::is_instance_valid(property_entry->node) && property_entry->node->get_multiplayer_authority() == peer)
			{
				owned_properties.push_back(property_entry);
			}
		}
		_auth_properties[peer] = owned_properties;
	}

	const Vector<Ref<PropertyEntry>>& owned_properties = _auth_properties[peer];
	
	TypedArray<PropertyEntry> result;
	result.resize(owned_properties.size());
	for (int i = 0; i < owned_properties.size(); ++i) {
		result.set(i, owned_properties[i]);
	}
	return result;
}

void _PropertyConfig::_bind_methods() {
	ClassDB::bind_method(D_METHOD("clear"), &_PropertyConfig::clear);
	
	ClassDB::bind_method(D_METHOD("set_properties", "p_properties"), &_PropertyConfig::set_properties);
	ClassDB::bind_method(D_METHOD("set_properties_from_paths", "property_paths", "property_cache"), &_PropertyConfig::set_properties_from_paths);
	ClassDB::bind_method(D_METHOD("get_properties"), &_PropertyConfig::get_properties);
	ClassDB::bind_method(D_METHOD("get_owned_properties"), &_PropertyConfig::get_owned_properties);
	ClassDB::bind_method(D_METHOD("get_properties_owned_by", "peer"), &_PropertyConfig::get_properties_owned_by);

	ClassDB::bind_method(D_METHOD("set_local_peer_id", "id"), &_PropertyConfig::set_local_peer_id);
	ClassDB::bind_method(D_METHOD("get_local_peer_id"), &_PropertyConfig::get_local_peer_id);
	
	ADD_PROPERTY(PropertyInfo(Variant::INT, "local_peer_id"), "set_local_peer_id", "get_local_peer_id");
}