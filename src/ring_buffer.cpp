#include "ring_buffer.h"

using namespace godot;

Ref<_RingBuffer> _RingBuffer::new_(int p_capacity) 
{
	Ref<_RingBuffer> ref;
  ref.instantiate();
  if (p_capacity <= 0) 
		p_capacity = 1;
	ref->_capacity = p_capacity;
	ref->_data.resize(p_capacity);
	ref->_size = 0;
	ref->_head = 0;
	return ref;
}

void _RingBuffer::push(const Variant& item) 
{
	_data.set(_head, item);

	if (_size < _capacity) 
		_size++;

	_head = (_head + 1) % _capacity;
}

Array _RingBuffer::get_data() const 
{
	Array result;
	for (int i = 0; i < _size; ++i)
		result.push_back(_data[i]);
	return result;
}

int _RingBuffer::capacity() const
{
  return _capacity;
}

int _RingBuffer::size() const 
{
	return _size;
}

bool _RingBuffer::is_empty() const 
{
	return _size == 0;
}

void _RingBuffer::clear() 
{
	_size = 0;
	_head = 0;
	_data.fill(Variant());
}

void _RingBuffer::_bind_methods() 
{
	ClassDB::bind_static_method("_RingBuffer", D_METHOD("new_", "p_capacity"), &_RingBuffer::new_);
	
	ClassDB::bind_method(D_METHOD("push", "item"), &_RingBuffer::push);
	ClassDB::bind_method(D_METHOD("get_data"), &_RingBuffer::get_data);
	ClassDB::bind_method(D_METHOD("size"), &_RingBuffer::size);
	ClassDB::bind_method(D_METHOD("capacity"), &_RingBuffer::capacity);
	ClassDB::bind_method(D_METHOD("is_empty"), &_RingBuffer::is_empty);
	ClassDB::bind_method(D_METHOD("clear"), &_RingBuffer::clear);
}
