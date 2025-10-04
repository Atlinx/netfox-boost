#pragma once

#include <godot_cpp/templates/hash_map.hpp>

using namespace godot;

template <typename TKey, typename TValue,
		typename Hasher = HashMapHasherDefault,
		typename TKeyComparator = HashMapComparatorDefault<TKey>,
		typename TValueComparator = HashMapComparatorDefault<TValue>,
		typename TKeyAllocator = DefaultTypedAllocator<HashMapElement<TKey, TValue>>,
		typename TValueAllocator = DefaultTypedAllocator<HashMapElement<TValue, TKey>>>
class InternalBiMap {
private:
	// Map 1: TKey -> TValue
	HashMap<TKey, TValue, Hasher, TKeyComparator, TKeyAllocator> map_kv; 

	// Map 2: TValue -> TKey (the inverse map)
	HashMap<TValue, TKey, Hasher, TValueComparator, TValueAllocator> map_vk; 

public:
	InternalBiMap() = default;
	~InternalBiMap() = default;

	using ConstIterator = typename HashMap<TKey, TValue, Hasher, TKeyComparator, TKeyAllocator>::ConstIterator;
	ConstIterator begin() const 
	{
		return map_kv.begin();
	}
	ConstIterator end() const
	{
		return map_kv.end();
	}

	void insert(const TKey& k, const TValue& v) 
	{
		remove_by_key(k);
		remove_by_value(v);

		map_kv.insert(k, v);
		map_vk.insert(v, k);
	}

	bool remove_by_key(const TKey& k) 
	{
		if (map_kv.has(k)) 
		{
			const TValue& v = map_kv.get(k); 
			map_kv.remove(map_kv.find(k));
			map_vk.remove(map_vk.find(v));
			return true;
		}
		return false;
	}

	bool remove_by_value(const TValue& v) 
	{
		if (map_vk.has(v)) 
		{
			const TKey& k = map_vk.get(v);
			map_kv.remove(map_kv.find(k));
			map_vk.remove(map_vk.find(v));
			return true;
		}
		return false;
	}

	void clear() 
	{
		map_kv.clear();
		map_vk.clear();
	}

	const TValue& get_by_key(const TKey& k) const 
	{
		return map_kv.get(k); 
	}
	
	const TKey& get_by_value(const TValue& v) const 
	{
		return map_vk.get(v);
	}
	
	int size() const 
	{
		return map_kv.size();
	}

	bool is_empty() const 
	{
		return map_kv.size() == 0;
	}

	bool has_key(const TKey& k) const 
	{
		return map_kv.has(k);
	}

	bool has_value(const TValue& v) const 
	{
		return map_vk.has(v);
	}
};