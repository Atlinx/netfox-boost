#pragma once

#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

class _Set : public RefCounted {
	GDCLASS(_Set, RefCounted);

private:
	HashSet<Variant, VariantHasher> _data;

protected:
	static void _bind_methods();

public:
	_Set() = default;
	~_Set() override = default;

	static Ref<_Set> of(const Array& p_items);
	static Ref<_Set> new_();

	void add(const Variant& p_value);
	bool has(const Variant& p_value) const;
	int size() const;
	bool is_empty() const;
	bool erase(const Variant& p_value);
	void clear();
	Array values() const;
	Variant min() const;
	Variant max() const;
	bool equals(const Ref<_Set>& p_other) const;
	
	bool _iter_init(Array p_iter);
	bool _iter_next(Array p_iter);
	Variant _iter_get(Array p_iter) const;

	String _to_string() const;
};