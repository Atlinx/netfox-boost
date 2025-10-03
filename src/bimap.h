#pragma once

#include <godot_cpp/templates/hash_map.hpp>

using namespace godot;

template <typename Key, typename Value>
class BiMap {
private:
	// Map 1: Key -> Value
	HashMap<Key, Value> map_kv; 

	// Map 2: Value -> Key (the inverse map)
	HashMap<Value, Key> map_vk; 

public:
	BiMap() = default;
	~BiMap() override = default;

	void insert(const Key& k, const Value& v) {
		// Ensure one-to-one constraint: remove existing entries first
		remove_by_key(k);
		remove_by_value(v);

		map_kv.insert(k, v);
		map_vk.insert(v, k);
	}

	void remove_by_key(const Key& k) {
		if (map_kv.has(k)) {
			// Retrieve value before erasing the key from map_kv
			const Value& v = map_kv.get(k); 
			map_vk.remove(v);
			map_kv.remove(k);
		}
	}

	void remove_by_value(const Value& v) {
		if (map_vk.has(v)) {
			// Retrieve key before erasing the value from map_vk
			const Key& k = map_vk.get(v);
			map_kv.remove(k);
			map_vk.remove(v);
		}
	}

	// Lookup functions
	const Value& get_by_key(const Key& k) const {
		// Warning: This assumes the key exists, similar to HashMap::get(). 
		// Use has_key() first for safety.
		return map_kv.get(k); 
	}
	
	const Key& get_by_value(const Value& v) const {
		// Warning: This assumes the value exists. Use has_value() first for safety.
		return map_vk.get(v);
	}
	
	// New utility functions:

	/**
	 * @brief Returns the number of elements (key-value pairs) in the map.
	 */
	int size() const {
		// Both maps should always have the same size.
		return map_kv.size();
	}

	/**
	 * @brief Checks if the map contains the specified key.
	 */
	bool has_key(const Key& k) const {
		return map_kv.has(k);
	}

	/**
	 * @brief Checks if the map contains the specified value.
	 */
	bool has_value(const Value& v) const {
		return map_vk.has(v);
	}
};