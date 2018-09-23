#ifndef SDLEX_HASHMAP_H
#define SDLEX_HASHMAP_H

#define PRIME1 0xbe1f14b1
#define PRIME2 0xb4b82e39
#define PRIME3 0xced1c241

#define DEFAULT_LOAD_FACTOR 0.8f
#define DEFAULT_SIZE 39

typedef struct CuckooHashMap {
	int Size;
	int _capacity;
	int _stashSize;
	int _stashCapacity;
	int _pushIterations;
	void ** _keyTable;
	void ** _valueTable;

	int _hashShift;
	int _mask;
	int _threshold;

	float _loadFactor;

	int(*HashFunc) (void * key);
	int(*EqualFunc) (void * key1, void * key2);

	int AutoFreeWhenRemove;
} CuckooHashMap;

/// Create an empty Hashmap.
CuckooHashMap * create_cuckoo_hashmap();

/// Create an empty Hashmap that pointers inside are automatically freed when destroy/removed.
CuckooHashMap * create_autofree_cuckoo_hashmap();

/// Create an empty Hashmap with the given parameters.
CuckooHashMap * create_cuckoo_hashmap_p(int initialCapacity, float loadFactor, int autoFree, int(*hash_func) (void * key), int(*equal_func) (void * key1, void * key2));

/// Destroy a Hashmap and release all resources related.
void destroy_cuckoo_hashmap(CuckooHashMap * map_obj);

/// Get the value of the entry specified by @param key in the Hashmap.
void * get_cuckoo_hashmap(CuckooHashMap * map_obj, void * key);

/// Insert or update the value of the entry specified by @param key with @param value.
/// @returns The old value of the key in the map, or NULL.
void * put_cuckoo_hashmap(CuckooHashMap * map_obj, void * key, void * value);

/// Remove the entry specified by @param key.
/// @returns The value of the key in the map, or NULL if key does not exist.
void * remove_from_cuckoo_hashmap(CuckooHashMap * map_obj, void * key);
#endif
