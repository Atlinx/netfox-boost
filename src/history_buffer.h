#pragma once

#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>

using namespace godot;

class _HistoryBuffer : public RefCounted {
	GDCLASS(_HistoryBuffer, RefCounted);
public:

// Maps ticks (int) to arbitrary data

protected:
	HashMap<int, Variant> _buffer;

public:
	Variant get_snapshot(int tick);
	void set_snapshot(int tick, Variant data);
	Dictionary get_buffer();
	int get_earliest_tick();
	int get_latest_tick();
	int get_closest_tick(int tick);
	Variant get_history(int tick);
	void trim(int earliest_tick_to_keep);
	void clear();
	int size();
	bool is_empty();
	bool has(int tick);
	Array ticks();
	void erase(int tick);
	static void _bind_methods();
};