//  Author:  Noah Himed
//  Date:    19 March 2020
//  Summary: Defines and implements a hash map that doubles
//           in size once a pre-defined load factor is met.

#ifndef EXPANDABLE_HASH_MAP
#define EXPANDABLE_HASH_MAP

#include <vector>
#include <list>
#include <functional>
#include <iostream>

const int INITIAL_SIZE = 8;

template<typename KeyType, typename ValueType>
class ExpandableHashMap
{
public:
	ExpandableHashMap(double max_load_factor = 0.5);
	~ExpandableHashMap();
	void Reset();
	int Size() const;
    // Attempts to add value-key pair into map, does nothing if key is
    // already in the map
	void Associate(const KeyType& key, const ValueType& value);

	  // for a map that can't be modified, return a pointer to const ValueType
	const ValueType* Find(const KeyType& key) const;

	  // for a modifiable map, return a pointer to modifiable ValueType
	ValueType* Find(const KeyType& key)
	{
		return const_cast<ValueType*>(const_cast<const ExpandableHashMap*>(this)->Find(key));
	}
    
	  // C++11 syntax for preventing copying and assignment
	ExpandableHashMap(const ExpandableHashMap&) = delete;
	ExpandableHashMap& operator=(const ExpandableHashMap&) = delete;

private:
    struct PAIR
    {
        PAIR(KeyType key, ValueType val): m_key(key), m_val(val) {}
        KeyType m_key;
        ValueType m_val;
    };
    
    std::vector<std::list<PAIR>*> m_map;
    int m_num_pairs;
    int m_size;
    double m_max_load;
    
    // returns true if pair was added to map, false if key was already in the map and the pair was updated
    bool AddPairToMap(std::vector<std::list<PAIR>*> &map, KeyType key, ValueType val, bool copying=false);
};

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::ExpandableHashMap(double max_load_factor)
:m_map(INITIAL_SIZE, nullptr)
{
    m_num_pairs = 0;
    m_size = INITIAL_SIZE;
    m_max_load = max_load_factor;
    
}

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::~ExpandableHashMap()
{
    Reset();
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::Reset()
{
    for(int i = 0; i < m_size; i++)
        delete m_map[i];
    m_map.resize(INITIAL_SIZE, nullptr);
}

template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType, ValueType>::Size() const
{
    return m_num_pairs;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::Associate(const KeyType& key, const ValueType& value)
{
    // attempt to add value to map
    if(AddPairToMap(m_map, key, value))
    {
        // if a pair was added, and map wasn't just updated
        m_num_pairs++;
    
        // if number of buckets filled causes load to go over pre-defined maxLoad
        if(double(m_num_pairs)/double(m_size) > m_max_load)
        {
            // create map with two times current number of buckets
            std::vector<std::list<PAIR>*> new_map(2*m_size, nullptr);
            // iterate through all values in the old map
            for(int bucket_num = 0; bucket_num < m_size; bucket_num++)
                // if the bucket is filled, reallocate all key-value pairs to new map
                if(m_map[bucket_num] != nullptr)
                    for(auto pair = m_map[bucket_num]->begin(); pair != m_map[bucket_num]->end(); pair++)
                        AddPairToMap(new_map, pair->m_key, pair->m_val, true);
            
            // delete all buckets in current map
            Reset();
            // replace the current map with the new map
            m_map = new_map;
            m_size *= 2;
        }
    }
}


template<typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType, ValueType>::Find(const KeyType& key) const
{
    // go through each pair, and if a match is found for passed in key,
    // return a pointer to the corresponding value
    unsigned int hasher(const KeyType& k);
    unsigned int bucket_num = hasher(key);
    bucket_num %= m_map.size();
    if(m_map[bucket_num] != nullptr)
        for(auto bucket_it = m_map[bucket_num]->begin(); bucket_it != m_map[bucket_num] -> end(); bucket_it++)
            if(bucket_it->m_key == key)
                return &(bucket_it->m_val);
    
    return nullptr;
}

template<typename KeyType, typename ValueType>
bool ExpandableHashMap<KeyType, ValueType>::AddPairToMap(std::vector<std::list<PAIR>*> &map,
                                                         KeyType key, ValueType val,
                                                         bool copying)
{
    unsigned int hasher(const KeyType& k);
    unsigned int bucket_num = hasher(key);
    bucket_num %= map.size();
    
    // if the bucket is filled, check if a match for passed in key can be found
    if(map[bucket_num] != nullptr)
    {
        if(!copying)
        {
            for(auto pair = map[bucket_num] -> begin(); pair != map[bucket_num] -> end(); pair++)
            {
                // if a match is found, update corresponding value and
                // return to indicate new pair wasn't added
                if(key == pair->m_key)
                {
                    pair->m_val = val;
                    return false;
                }
            }
        }
    }
    else
        // if the bucket isn't filled, add new list
        map[bucket_num] = new std::list<PAIR>;
 
    PAIR new_pair(key, val);
    map[bucket_num]->push_back(new_pair);

    // indicate a new pair has been added
    return true;
}


#endif // EXPANDABLE_HASH_MAP
