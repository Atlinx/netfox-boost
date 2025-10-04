#include "bimap.h"

#include <godot_cpp/core/object.hpp>

Ref<_BiMap> _BiMap::of(const Dictionary& p_dict) 
{
	Ref<_BiMap> ref;
	ref.instantiate();
	for (auto key : p_dict.keys())
		ref->put(key, p_dict[key]);
	return ref;
}

Ref<_BiMap> _BiMap::new_() 
{
	Ref<_BiMap> ref;
  ref.instantiate();
  return ref;
}

void _BiMap::put(const Variant& p_key, const Variant& p_value) 
{
	_data.insert(p_key, p_value);
}

Variant _BiMap::get_by_key(const Variant& p_key) const
{
	return _data.get_by_key(p_key);
}

Variant _BiMap::get_by_value(const Variant& p_value) const
{
	return _data.get_by_value(p_value);
}

bool _BiMap::has_key(const Variant& p_key) const 
{
	return _data.has_key(p_key);
}

bool _BiMap::has_value(const Variant& p_value) const 
{
	return _data.has_value(p_value);
}

bool _BiMap::is_empty() const 
{
	return _data.is_empty();
}

int _BiMap::size() const 
{
	return _data.size();
}

bool _BiMap::erase_key(const Variant& p_key) 
{
	return _data.remove_by_key(p_key);
}

bool _BiMap::erase_value(const Variant& p_value) 
{
	return _data.remove_by_value(p_value);
}

void _BiMap::clear() 
{
	_data.clear();
}

Array _BiMap::keys() const 
{
  Array res;
  res.resize(_data.size());
  int i = 0;
  for (auto key_value : _data) 
	{
		res[i] = key_value.key;
    ++i;
	}
	return res;
}

Array _BiMap::values() const 
{
  Array res;
  res.resize(_data.size());
  int i = 0;
  for (auto key_value : _data) 
	{
		res[i] = key_value.value;
    ++i;
	}
	return res;
}

String _BiMap::_to_string() const 
{
	Dictionary dict;
	for (auto key_value : _data)
		dict[key_value.key] = key_value.value;
	return "BiMap: " + vformat("%s", dict);
}

bool _BiMap::_iter_init(Array p_iter) const
{
	if (_data.is_empty()) {
		return false;
	}
	Ref<_BiMapIterator> iter;
	iter.instantiate();
	iter->iterator = _data.begin();
	p_iter[0] = iter;
  return true;
}

bool _BiMap::_iter_next(Array p_iter) const
{
	Ref<_BiMapIterator> iterator = p_iter[0];
	++iterator->iterator;
	if (iterator->iterator)
		return true;
  return false;
}

Variant _BiMap::_iter_get(Ref<_BiMapIterator> p_iter) const 
{
	return p_iter->iterator->key;
}

void _BiMap::_bind_methods() 
{
	ClassDB::bind_static_method("_BiMap", D_METHOD("of", "items"), &_BiMap::of);

	ClassDB::bind_method(D_METHOD("put", "p_key", "p_value"), &_BiMap::put);
	ClassDB::bind_method(D_METHOD("get_by_key", "p_key"), &_BiMap::get_by_key);
	ClassDB::bind_method(D_METHOD("get_by_value", "p_value"), &_BiMap::get_by_value);
	ClassDB::bind_method(D_METHOD("has_key", "p_key"), &_BiMap::has_key);
	ClassDB::bind_method(D_METHOD("has_value", "p_value"), &_BiMap::has_value);
	ClassDB::bind_method(D_METHOD("is_empty"), &_BiMap::is_empty);
	ClassDB::bind_method(D_METHOD("erase_key", "p_key"), &_BiMap::erase_key);
	ClassDB::bind_method(D_METHOD("erase_value", "p_value"), &_BiMap::erase_value);
	ClassDB::bind_method(D_METHOD("size"), &_BiMap::size);
	ClassDB::bind_method(D_METHOD("clear"), &_BiMap::clear);
	ClassDB::bind_method(D_METHOD("keys"), &_BiMap::keys);
	ClassDB::bind_method(D_METHOD("values"), &_BiMap::values);
	
	ClassDB::bind_method(D_METHOD("_iter_init", "iter"), &_BiMap::_iter_init);
	ClassDB::bind_method(D_METHOD("_iter_next", "iter"), &_BiMap::_iter_next);
	ClassDB::bind_method(D_METHOD("_iter_get", "iter"), &_BiMap::_iter_get);

	ClassDB::bind_method(D_METHOD("_to_string"), &_BiMap::_to_string);
}

void _BiMapIterator::_bind_methods()
{

}