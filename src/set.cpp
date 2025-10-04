#include "set.h"

#include <godot_cpp/core/object.hpp>

Ref<_Set> _Set::new_() {
	Ref<_Set> ref;
  ref.instantiate();
  return ref;
}

Ref<_Set> _Set::of(const Array& p_items) {
	Ref<_Set> result = memnew(_Set);
	for (int i = 0; i < p_items.size(); ++i) {
		result->add(p_items[i]);
	}
	return result;
}

void _Set::add(const Variant& p_value) {
	_data.insert(p_value);
}

bool _Set::has(const Variant& p_value) const {
	return _data.has(p_value);
}

int _Set::size() const {
	return _data.size();
}

bool _Set::is_empty() const {
	return _data.is_empty();
}

bool _Set::erase(const Variant& p_value) {
	return _data.erase(p_value);
}

void _Set::clear() {
	_data.clear();
}

Array _Set::values() const {
  Array res;
  res.resize(_data.size());
  int i = 0;
  for (auto data : _data)
    res[i] = data;
    ++i;
	return res;
}

Variant _Set::min() const {
  if (_data.is_empty()) {
    return Variant();
  }
  Variant smallest = *_data.begin();
  for (auto data : _data)
    if (smallest < (Variant) data)
      smallest = data;
	return smallest;
}

Variant _Set::max() const {
  if (_data.is_empty()) {
    return Variant();
  }
  Variant largest = *_data.begin();
  for (auto data : _data)
    if ((Variant) data < largest)
      largest = data;
	return largest;
}

bool _Set::equals(const Ref<_Set>& p_other) const {
	if (p_other.is_null())
		return false;

	if (_data.size() != p_other->_data.size())
		return false;

	for (auto data : _data) {
		if (!p_other->_data.has(data))
			return false;
	}

	return true;
}

String _Set::_to_string() const {
	return "Set: [" + String(", ").join(values()) + "]";
}

bool _Set::_iter_init(Array p_iter) {
	if (_data.is_empty()) {
		return false;
	}
	p_iter[0] = 0;
  return true;
}

bool _Set::_iter_next(Array p_iter) {
  int curr_idx = p_iter[0];
	if (curr_idx >= 0 && curr_idx < _data.size()) {
		p_iter[0] = curr_idx + 1;
    return true;
	}
  p_iter[0] = -1;
  return false;
}

Variant _Set::_iter_get(Array p_iter) const {
  int curr_idx = p_iter[0];
  if (curr_idx >= 0 && curr_idx < _data.size()) {
    auto it = _data.begin();
    while (curr_idx > 0) {
      curr_idx--;
      ++it;
    }
    return *it;
  }
  return Variant();
}

void _Set::_bind_methods() {
	ClassDB::bind_static_method("_Set", D_METHOD("of", "items"), &_Set::of);
	ClassDB::bind_static_method("_Set", D_METHOD("new"), &_Set::new_);

	ClassDB::bind_method(D_METHOD("add", "value"), &_Set::add);
	ClassDB::bind_method(D_METHOD("has", "value"), &_Set::has);
	ClassDB::bind_method(D_METHOD("size"), &_Set::size);
	ClassDB::bind_method(D_METHOD("is_empty"), &_Set::is_empty);
	ClassDB::bind_method(D_METHOD("erase", "value"), &_Set::erase);
	ClassDB::bind_method(D_METHOD("clear"), &_Set::clear);
	ClassDB::bind_method(D_METHOD("values"), &_Set::values);
	ClassDB::bind_method(D_METHOD("min"), &_Set::min);
	ClassDB::bind_method(D_METHOD("max"), &_Set::max);
	ClassDB::bind_method(D_METHOD("equals", "other"), &_Set::equals);
	
	ClassDB::bind_method(D_METHOD("_iter_init", "iter"), &_Set::_iter_init);
	ClassDB::bind_method(D_METHOD("_iter_next", "iter"), &_Set::_iter_next);
	ClassDB::bind_method(D_METHOD("_iter_get", "iter"), &_Set::_iter_get);

	ClassDB::bind_method(D_METHOD("_to_string"), &_Set::_to_string);
}