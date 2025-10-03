#pragma once

#include "history_buffer.h"
#include "property_snapshot.h"

#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>

using namespace godot;

class _PropertyHistoryBuffer : public _HistoryBuffer {
	GDCLASS(_PropertyHistoryBuffer, _HistoryBuffer);

	protected:
	static void _bind_methods();
	
public:
	_PropertyHistoryBuffer() = default;
	~_PropertyHistoryBuffer() override = default;

	Ref<_PropertySnapshot> get_snapshot(int tick);
	void set_snapshot(int tick, Variant data);
	Ref<_PropertySnapshot> get_history(int tick);
	void trim(int earliest_tick_to_keep = -1);
	void merge(Ref<_PropertySnapshot> data, int tick);
};