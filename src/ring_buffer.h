#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/templates/vector.hpp>

using namespace godot;

class _RingBuffer : public RefCounted {
	GDCLASS(_RingBuffer, RefCounted)

private:
	Vector<Variant> _data;
	int _capacity = 0;
	int _size = 0;
	int _head = 0;

protected:
	static void _bind_methods();

public:
	_RingBuffer() = default;
	~_RingBuffer() override = default;

	static Ref<_RingBuffer> new_(int p_capacity); 
  void push(const Variant& item);
	Array get_data() const;
	int size() const;
	int capacity() const;
	bool is_empty() const;
	void clear();
};