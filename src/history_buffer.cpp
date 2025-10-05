#include "history_buffer.h"

#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <limits>

Variant _HistoryBuffer::get_snapshot(int tick)
{
	if(_buffer.has(tick))
		return _buffer[tick];
	else
		return Variant();
}

void _HistoryBuffer::set_snapshot(int tick, Variant data)
{
	_buffer[tick] = data;
}

Dictionary _HistoryBuffer::get_buffer()
{
	Dictionary result;
	for (auto key_value : _buffer) 
		result[key_value.key] = key_value.value;
	return result;
}

int _HistoryBuffer::get_earliest_tick()
{
	if (_buffer.is_empty())
		return -1;

	int min_key = _buffer.begin()->key;
	for (auto key_value : _buffer)
	{
		if (key_value.key < min_key)
			min_key = key_value.key;
	}
	return min_key;
}

int _HistoryBuffer::get_latest_tick()
{
	if (_buffer.is_empty())
		return -1;

	int max_key = _buffer.begin()->key;
	for (auto key_value : _buffer)
	{
		if (key_value.key > max_key)
			max_key = key_value.key;
	}
	return max_key;
}

int _HistoryBuffer::get_closest_tick(int tick)
{
	if (_buffer.has(tick))
		return tick;

	if (_buffer.is_empty())
		return -1;

	int earliest_tick = get_earliest_tick();
	if (tick < earliest_tick)
		return earliest_tick;

	int latest_tick = get_latest_tick();
	if (tick > latest_tick)
		return latest_tick;

	int max_key = _buffer.begin()->key;
	for (auto key_value : _buffer)
	{
		if (key_value.key < tick && key_value.key > max_key)
				max_key = key_value.key;
	}

	return max_key;
}

Variant _HistoryBuffer::get_history(int tick)
{
	int closest_tick = get_closest_tick(tick);
	// print_line("_HistoryBuffer::get_history: tick: ", tick, " closest tick: ", closest_tick);
	if (_buffer.has(closest_tick))
		return _buffer[closest_tick];
	// print_line("  '- HISTORY BUFFER CANNOT FIND ENTRY");
	return Variant();
}

void _HistoryBuffer::trim(int earliest_tick_to_keep)
{
	Vector<int> ticks_to_erase;
	for (auto key_value : _buffer)
	{
		if (key_value.key < earliest_tick_to_keep)
			ticks_to_erase.push_back(key_value.key);
	}

	for (int tick : ticks_to_erase)
		_buffer.erase(tick);
}

void _HistoryBuffer::clear()
{
	_buffer.clear();
}

int _HistoryBuffer::size()
{
	return _buffer.size();
}

bool _HistoryBuffer::is_empty()
{
	return _buffer.is_empty();
}

bool _HistoryBuffer::has(int tick)
{
	return _buffer.has(tick);
}

Array _HistoryBuffer::ticks()
{
	// Convert internal Vector<int> keys to Godot Array for the API
	Array result;
	for (auto key_value : _buffer)
		result.append(key_value.key);

	return result;
}

void _HistoryBuffer::erase(int tick)
{
	_buffer.erase(tick);
}

void _HistoryBuffer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_snapshot", "tick"), &_HistoryBuffer::get_snapshot);
	ClassDB::bind_method(D_METHOD("set_snapshot", "tick", "data"), &_HistoryBuffer::set_snapshot);
	ClassDB::bind_method(D_METHOD("get_buffer"), &_HistoryBuffer::get_buffer);
	ClassDB::bind_method(D_METHOD("get_earliest_tick"), &_HistoryBuffer::get_earliest_tick);
	ClassDB::bind_method(D_METHOD("get_latest_tick"), &_HistoryBuffer::get_latest_tick);
	ClassDB::bind_method(D_METHOD("get_closest_tick", "tick"), &_HistoryBuffer::get_closest_tick);
	ClassDB::bind_method(D_METHOD("get_history", "tick"), &_HistoryBuffer::get_history);
	ClassDB::bind_method(D_METHOD("trim", "earliest_tick_to_keep"), &_HistoryBuffer::trim);
	ClassDB::bind_method(D_METHOD("clear"), &_HistoryBuffer::clear);
	ClassDB::bind_method(D_METHOD("size"), &_HistoryBuffer::size);
	ClassDB::bind_method(D_METHOD("is_empty"), &_HistoryBuffer::is_empty);
	
	// Has and Erase are changed to take 'int' directly for C++ usage, but bound to Variant for GDScript
	ClassDB::bind_method(D_METHOD("has", "tick"), &_HistoryBuffer::has);
	ClassDB::bind_method(D_METHOD("ticks"), &_HistoryBuffer::ticks);
	ClassDB::bind_method(D_METHOD("erase", "tick"), &_HistoryBuffer::erase);
}