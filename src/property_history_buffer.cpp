
#include "property_history_buffer.h"
#include "utils.h"

#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/node_path.hpp>

Ref<_PropertySnapshot> _PropertyHistoryBuffer::get_snapshot(int tick)
{
	if(_buffer.has(tick))
		return _buffer[tick];
	else
		return _PropertySnapshot::_new();
}

void _PropertyHistoryBuffer::set_snapshot(int tick, Variant data)
{
	if(Object::cast_to<Dictionary>(data))
	{
		Ref<_PropertySnapshot> snapshot = _PropertySnapshot::from_dictionary(data);
		_HistoryBuffer::set_snapshot(tick, snapshot);
	}
	else if(Object::cast_to<_PropertySnapshot>(data))
		_HistoryBuffer::set_snapshot(tick, data);
	else
		UtilityFunctions::push_error(vformat("Data not a PropertSnapshot! %s", data));
}

Ref<_PropertySnapshot> _PropertyHistoryBuffer::get_history(int tick)
{
	Variant snapshot = _HistoryBuffer::get_history(tick);

	return ( snapshot ? snapshot : _PropertySnapshot::_new() );
}

void _PropertyHistoryBuffer::trim(int earliest_tick_to_keep)
{
	if (earliest_tick_to_keep == -1) 
	{
		auto autoload = Utils::get_autoload("NetworkRollback");
		if (autoload != nullptr)
			earliest_tick_to_keep = autoload->get("history_start");
	}
	_HistoryBuffer::trim(earliest_tick_to_keep);
}

void _PropertyHistoryBuffer::merge(Ref<_PropertySnapshot> data, int tick)
{
	set_snapshot(tick, get_snapshot(tick)->merge(data));
}

void _PropertyHistoryBuffer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_snapshot", "tick"), &_PropertyHistoryBuffer::get_snapshot);
	ClassDB::bind_method(D_METHOD("set_snapshot", "tick", "data"), &_PropertyHistoryBuffer::set_snapshot);
	ClassDB::bind_method(D_METHOD("get_history", "tick"), &_PropertyHistoryBuffer::get_history);
	ClassDB::bind_method(D_METHOD("trim", "earliest_tick_to_keep"), &_PropertyHistoryBuffer::trim);
	ClassDB::bind_method(D_METHOD("merge", "data", "tick"), &_PropertyHistoryBuffer::merge);
}

