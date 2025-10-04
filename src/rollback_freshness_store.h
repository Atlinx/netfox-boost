#pragma once

#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

using namespace godot;

class RollbackFreshnessStore : public RefCounted {
	GDCLASS(RollbackFreshnessStore, RefCounted);

protected:
	HashMap<int, HashMap<Node*, bool>> _data;

	static void _bind_methods();

public:
	static Ref<RollbackFreshnessStore> new_();

	RollbackFreshnessStore() = default;
	~RollbackFreshnessStore() override = default;

	bool is_fresh(Node* node, int tick);
	void notify_processed(Node* node, int tick);
	void trim();
	void clear();
};