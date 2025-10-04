#include "rollback_freshness_store.h"
#include "utils.h"

#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/variant.hpp>

Ref<RollbackFreshnessStore> RollbackFreshnessStore::new_()
{
	Ref<RollbackFreshnessStore> ref;
	ref.instantiate();
	return ref;
}

bool RollbackFreshnessStore::is_fresh(Node* node, int tick)
{
	if(!_data.has(tick))
	{
		return true;
	}

	if(!_data[tick].has(node))
	{
		return true;
	}

	return false;
}

void RollbackFreshnessStore::notify_processed(Node* node, int tick)
{
	if(!_data.has(tick))
		_data[tick] = HashMap<Node*, bool>();

	_data[tick][node] = true;
}

void RollbackFreshnessStore::trim()
{
	while(!_data.is_empty())
	{
		int earliest_tick = 0;
		bool first = true;
		for(const KeyValue<int, HashMap<Node*, bool>>& E : _data)
		{
			if (first || E.key < earliest_tick)
			{
				earliest_tick = E.key;
				first = false;
			}
		}

		if(earliest_tick < (int)Utils::get_autoload("NetworkRollback")->get("history_start"))
		{
			_data.erase(earliest_tick);
		}
		else
		{
			break;
		}
	}
}

void RollbackFreshnessStore::clear()
{
	_data.clear();
}

void RollbackFreshnessStore::_bind_methods() {
	ClassDB::bind_static_method("RollbackFreshnessStore", D_METHOD("new"), &RollbackFreshnessStore::new_);
	ClassDB::bind_method(D_METHOD("is_fresh", "node", "tick"), &RollbackFreshnessStore::is_fresh);
	ClassDB::bind_method(D_METHOD("notify_processed", "node", "tick"), &RollbackFreshnessStore::notify_processed);
	ClassDB::bind_method(D_METHOD("trim"), &RollbackFreshnessStore::trim);
	ClassDB::bind_method(D_METHOD("clear"), &RollbackFreshnessStore::clear);
}