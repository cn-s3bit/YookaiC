#ifndef SDLEX_HASHMAP_H
#define SDLEX_HASHMAP_H
#include <string.h>
#include "MemoryPool.h"

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
	void(*FreeKeyFunc) (void * key);

	int AutoFreeWhenRemove;
} CuckooHashMap;

/// Create an empty Hashmap.
CuckooHashMap * create_cuckoo_hashmap();

/// Create an empty Hashmap that pointers inside are automatically freed when destroy/removed.
CuckooHashMap * create_autofree_cuckoo_hashmap();

/// Create an empty Hashmap with the given parameters.
CuckooHashMap * create_cuckoo_hashmap_p(int initialCapacity, float loadFactor, int autoFree, int(*hash_func) (void * key), int(*equal_func) (void * key1, void * key2), void(*free_key_func) (void * key));

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

typedef CuckooHashMap IntIntCuckooHashMap;

IntIntCuckooHashMap * create_intint_cuckoo_hashmap();
void put_intint_cuckoo_hashmap(IntIntCuckooHashMap * map_obj, int key, int value);
int get_intint_cuckoo_hashmap(IntIntCuckooHashMap * map_obj, int key);
int remove_from_intint_cuckoo_hashmap(IntIntCuckooHashMap * map_obj, int key);
int sdlex_hash_int(void * pt);
int sdlex_equal_int(void * pt1, void * pt2);

#define CODEGEN_HASHMAP_DEFAULT_HASHFUNC(FUNCNAME, KEYTYPE) \
	static int FUNCNAME(void * key) {\
		int hash = 0;\
		for (unsigned i = 0; i < sizeof(KEYTYPE); i++) {\
			hash = hash * 31 + (*(((char *)key) + i))\
		}\
		return hash;\
	}

#define CODEGEN_HASHMAP_DEFAULT_EQUALFUNC(FUNCNAME, KEYTYPE) \
	static int FUNCNAME(void * pt1, void * pt2) {\
		if (pt1 == NULL && pt2 != NULL)\
			return 0;\
		if (pt1 != NULL && pt2 == NULL)\
			return 0;\
		return memcmp(pt1, pt2, sizeof(KEYTYPE)) == 0;\
	}

#define CODEGEN_CUCKOO_HASHMAP(POSTFIX, KEYTYPE, VALUETYPE, HASH_FUNC, EQUAL_FUNC, FREE_KEY_FUNC, FREE_VALUE_FUNC) \
	static CuckooHashMap * create_##POSTFIX() {\
		return create_cuckoo_hashmap_p(DEFAULT_SIZE, DEFAULT_LOAD_FACTOR, 1, HASH_FUNC, EQUAL_FUNC, FREE_KEY_FUNC);\
	}\
\
	static void put_##POSTFIX(CuckooHashMap * map_obj, KEYTYPE key, VALUETYPE value) {\
		KEYTYPE * pkey = memorypool_malloc(sizeof(KEYTYPE));\
		VALUETYPE * pvalue = memorypool_malloc(sizeof(VALUETYPE));\
		*pkey = key;\
		*pvalue = value;\
		VALUETYPE * fetched = put_cuckoo_hashmap(map_obj, pkey, pvalue);\
		if (fetched) {\
			FREE_VALUE_FUNC(fetched);\
		}\
	}\
\
	static VALUETYPE get_##POSTFIX(CuckooHashMap * map_obj, KEYTYPE key) {\
		KEYTYPE * pkey = memorypool_malloc(sizeof(KEYTYPE));\
		*pkey = key;\
		VALUETYPE * fetched = get_cuckoo_hashmap(map_obj, pkey);\
		FREE_KEY_FUNC(pkey);\
		return *fetched;\
	}\
\
	static VALUETYPE remove_from_##POSTFIX(CuckooHashMap * map_obj, KEYTYPE key) {\
		KEYTYPE * pkey = memorypool_malloc(sizeof(KEYTYPE));\
		*pkey = key;\
		VALUETYPE * fetched = remove_from_cuckoo_hashmap(map_obj, pkey);\
		FREE_KEY_FUNC(pkey);\
		VALUETYPE result = *fetched;\
		FREE_VALUE_FUNC(fetched);\
		return result;\
	}
#endif
