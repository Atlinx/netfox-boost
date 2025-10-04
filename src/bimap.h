#pragma once

#include "internal_bimap.h"

#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/variant.hpp>

using namespace godot;

class _BiMapIterator : public RefCounted {
	GDCLASS(_BiMapIterator, RefCounted);

	protected:
		static void _bind_methods();

	public:
		_BiMapIterator() = default;
		~_BiMapIterator() override = default;

		InternalBiMap<Variant, Variant, VariantHasher>::ConstIterator iterator;
};

class _BiMap : public RefCounted {
	GDCLASS(_BiMap, RefCounted);

private:
	InternalBiMap<Variant, Variant, VariantHasher> _data;

protected:
	static void _bind_methods();

public:
	_BiMap() = default;
	~_BiMap() override = default;

	static Ref<_BiMap> of(const Dictionary& p_items);
	static Ref<_BiMap> new_();

	void put(const Variant& p_key, const Variant& p_value);
	Variant get_by_key(const Variant& p_key) const;
	Variant get_by_value(const Variant& p_value) const;
	bool has_key(const Variant& p_key) const;
	bool has_value(const Variant& p_value) const;
	bool is_empty() const;
	bool erase_key(const Variant& p_key);
	bool erase_value(const Variant& p_value);
	int size() const;
	void clear();
	Array keys() const;
	Array values() const;
	
	bool _iter_init(Array p_iter) const;
	bool _iter_next(Array p_iter) const;
	Variant _iter_get(Ref<_BiMapIterator> p_iter) const;

	String _to_string() const;
};