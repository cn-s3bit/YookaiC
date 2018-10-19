#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef SDLEX_DEBUG_OUTPUT
#include <stdio.h>
#endif
#include "HashMap.h"

// Reference: https://github.com/libgdx/libgdx/blob/master/gdx/src/com/badlogic/gdx/utils/ObjectMap.java

inline int _sdlex_hashmap_nextpo2(int inp) {
	int base = 1;
	while (inp > base) {
		base <<= 1;
	}
	return base;
}

inline int _sdlex_hashmap_ror(int val, int size) {
	int res = val >> size;
	res |= val << (32 - size);
	return res;
	// https://blog.csdn.net/dfq12345/article/details/78177767?utm_source=copy
}

inline int _sdlex_hashmap_trailing_0(int i) {
	int y;
	if (i == 0) return 32;
	int n = 31;
	y = i << 16; if (y != 0) { n = n - 16; i = y; }
	y = i << 8; if (y != 0) { n = n - 8; i = y; }
	y = i << 4; if (y != 0) { n = n - 4; i = y; }
	y = i << 2; if (y != 0) { n = n - 2; i = y; }
	return n - _sdlex_hashmap_ror((i << 1), 31);
	// https://blog.csdn.net/tina_tian1/article/details/78364795?utm_source=copy
}

inline int hash2(CuckooHashMap * map_obj, int h) {
	h *= PRIME2;
	return (h ^ _sdlex_hashmap_ror(h, map_obj->_hashShift)) & map_obj->_mask;
}

inline int hash3(CuckooHashMap * map_obj, int h) {
	h *= PRIME3;
	return (h ^ _sdlex_hashmap_ror(h, map_obj->_hashShift)) & map_obj->_mask;
}

CuckooHashMap * create_cuckoo_hashmap() {
	return create_cuckoo_hashmap_p(DEFAULT_SIZE, DEFAULT_LOAD_FACTOR, 0, NULL, NULL, NULL);
}

CuckooHashMap * create_autofree_cuckoo_hashmap() {
	return create_cuckoo_hashmap_p(DEFAULT_SIZE, DEFAULT_LOAD_FACTOR, 1, NULL, NULL, NULL);
}

CuckooHashMap * create_cuckoo_hashmap_p(int initialCapacity, float loadFactor, int autoFree, int (*hash_func) (void * key), int(*equal_func) (void * key1, void * key2), void(*free_key_func) (void * key)) {
	if (initialCapacity <= 0 || loadFactor <= 0.0f || initialCapacity > (1 << 29)) {
#ifdef SDLEX_DEBUG_OUTPUT
#ifdef _MSC_VER
		printf_s("Hashmap Initialize Paramters Unaccepted: initialCapacity = %d, loadFactor = %f\n", initialCapacity, loadFactor);
#else
		printf("Hashmap Initialize Paramters Unaccepted: initialCapacity = %d, loadFactor = %f\n", initialCapacity, loadFactor);
#endif
#endif
		return NULL;
	}
	CuckooHashMap * map_obj = malloc(sizeof(CuckooHashMap));
	memset(map_obj, 0, sizeof(CuckooHashMap));
	register int realCapacity = _sdlex_hashmap_nextpo2((int)ceil(initialCapacity / loadFactor));
	map_obj->_capacity = realCapacity;
	map_obj->_loadFactor = loadFactor;
	map_obj->_threshold = (int)(realCapacity * loadFactor);
	map_obj->_mask = realCapacity - 1;
	map_obj->_hashShift = 31 - _sdlex_hashmap_trailing_0(realCapacity);
	map_obj->_stashCapacity = max(3, ((int)ceil(log(realCapacity))) * 2);
	map_obj->_pushIterations = max(min(realCapacity, 8), ((int)sqrt(realCapacity)) / 8);
	map_obj->HashFunc = hash_func;
	map_obj->EqualFunc = equal_func;
	map_obj->FreeKeyFunc = free_key_func;
	map_obj->AutoFreeWhenRemove = autoFree;

	int tableSize = sizeof(void *) * (realCapacity + map_obj->_stashCapacity);
	map_obj->_keyTable = malloc(tableSize);
	memset(map_obj->_keyTable, 0, tableSize);
	map_obj->_valueTable = malloc(tableSize);
	memset(map_obj->_valueTable, 0, tableSize);

	return map_obj;
}

void _sdlex_hashmap_resize(CuckooHashMap * map_obj, int newSize);
void _sdlex_hashmap_push(CuckooHashMap * map_obj, void * insertKey, void * insertValue, int index1, void * key1, int index2, void * key2, int index3, void * key3);

void _sdlex_hashmap_putresize(CuckooHashMap * map_obj, void * key, void * value) {
	// Check for empty buckets.
	int hashCode = map_obj->HashFunc ? map_obj->HashFunc(key) : (int)key;
	int index1 = hashCode & map_obj->_mask;
	void * key1 = map_obj->_keyTable[index1];
	if (key1 == NULL) {
		map_obj->_keyTable[index1] = key;
		map_obj->_valueTable[index1] = value;
		if (map_obj->Size++ >= map_obj->_threshold) _sdlex_hashmap_resize(map_obj, map_obj->_capacity << 1);
		return;
	}

	int index2 = hash2(map_obj, hashCode);
	void * key2 = map_obj->_keyTable[index2];
	if (key2 == NULL) {
		map_obj->_keyTable[index2] = key;
		map_obj->_valueTable[index2] = value;
		if (map_obj->Size++ >= map_obj->_threshold) _sdlex_hashmap_resize(map_obj, map_obj->_capacity << 1);
		return;
	}

	int index3 = hash3(map_obj, hashCode);
	void * key3 = map_obj->_keyTable[index3];
	if (key3 == NULL) {
		map_obj->_keyTable[index3] = key;
		map_obj->_valueTable[index3] = value;
		if (map_obj->Size++ >= map_obj->_threshold) _sdlex_hashmap_resize(map_obj, map_obj->_capacity << 1);
		return;
	}

	_sdlex_hashmap_push(map_obj, key, value, index1, key1, index2, key2, index3, key3);
}

void _sdlex_hashmap_resize(CuckooHashMap * map_obj, int newSize) {
	int oldEndIndex = map_obj->_capacity + map_obj->_stashSize;

	map_obj->_capacity = newSize;
	map_obj->_threshold = (int)(newSize * map_obj->_loadFactor);
	map_obj->_mask = newSize - 1;
	map_obj->_hashShift = 31 - _sdlex_hashmap_trailing_0(newSize);
	map_obj->_stashCapacity = max(3, (int)ceil(log(newSize)) * 2);
	map_obj->_pushIterations = max(min(newSize, 8), (int)sqrt(newSize) / 8);

	void ** oldKeyTable = map_obj->_keyTable;
	void ** oldValueTable = map_obj->_valueTable;

	int tableSize = sizeof(void *) * (newSize + map_obj->_stashCapacity);
	map_obj->_keyTable = malloc(tableSize);
	memset(map_obj->_keyTable, 0, tableSize);
	map_obj->_valueTable = malloc(tableSize);
	memset(map_obj->_valueTable, 0, tableSize);

	int oldSize = map_obj->Size;
	map_obj->Size = 0;
	map_obj->_stashSize = 0;
	if (oldSize > 0) {
		for (int i = 0; i < oldEndIndex; i++) {
			void * key = oldKeyTable[i];
			if (key != NULL) _sdlex_hashmap_putresize(map_obj, key, oldValueTable[i]);
		}
	}
	free(oldKeyTable);
	free(oldValueTable);
}

inline void _sdlex_hashmap_putstash(CuckooHashMap * map_obj, void * key, void * value) {
	if (map_obj->_stashSize == map_obj->_stashCapacity) {
		// Too many pushes occurred and the stash is full, increase the table size.
		_sdlex_hashmap_resize(map_obj, map_obj->_capacity << 1);
		_sdlex_hashmap_putresize(map_obj, key, value);
		return;
	}
	// Store key in the stash.
	int index = map_obj->_capacity + map_obj->_stashSize;
	map_obj->_keyTable[index] = key;
	map_obj->_valueTable[index] = value;
	map_obj->_stashSize++;
	map_obj->Size++;
}

void _sdlex_hashmap_push(CuckooHashMap * map_obj, void * insertKey, void * insertValue, int index1, void * key1, int index2, void * key2, int index3, void * key3) {
	void ** keyTable = map_obj->_keyTable;
	void ** valueTable = map_obj->_valueTable;
	int mask = map_obj->_mask;

	// Push keys until an empty bucket is found.
	void * evictedKey;
	void * evictedValue;
	int i = 0, pushIterations = map_obj->_pushIterations;
	int state = 0;
	do {
		// Replace the key and value for one of the hashes.
		switch (state++) {
		case 0:
			evictedKey = key1;
			evictedValue = valueTable[index1];
			keyTable[index1] = insertKey;
			valueTable[index1] = insertValue;
			break;
		case 1:
			evictedKey = key2;
			evictedValue = valueTable[index2];
			keyTable[index2] = insertKey;
			valueTable[index2] = insertValue;
			break;
		default:
			evictedKey = key3;
			evictedValue = valueTable[index3];
			keyTable[index3] = insertKey;
			valueTable[index3] = insertValue;
			break;
		}
		state = state > 2 ? 0 : state;

		// If the evicted key hashes to an empty bucket, put it there and stop.
		int hashCode = map_obj->HashFunc ? map_obj->HashFunc(evictedKey) : (int) evictedKey;
		index1 = hashCode & mask;
		key1 = keyTable[index1];
		if (key1 == NULL) {
			keyTable[index1] = evictedKey;
			valueTable[index1] = evictedValue;
			if (map_obj->Size++ >= map_obj->_threshold) _sdlex_hashmap_resize(map_obj, map_obj->_capacity << 1);
			return;
		}

		index2 = hash2(map_obj, hashCode);
		key2 = keyTable[index2];
		if (key2 == NULL) {
			keyTable[index2] = evictedKey;
			valueTable[index2] = evictedValue;
			if (map_obj->Size++ >= map_obj->_threshold) _sdlex_hashmap_resize(map_obj, map_obj->_capacity << 1);
			return;
		}

		index3 = hash3(map_obj, hashCode);
		key3 = keyTable[index3];
		if (key3 == NULL) {
			keyTable[index3] = evictedKey;
			valueTable[index3] = evictedValue;
			if (map_obj->Size++ >= map_obj->_threshold) _sdlex_hashmap_resize(map_obj, map_obj->_capacity << 1);
			return;
		}

		if (++i == pushIterations) break;

		insertKey = evictedKey;
		insertValue = evictedValue;
	} while (1);

	_sdlex_hashmap_putstash(map_obj, evictedKey, evictedValue);
}

#define EQUALS_FUNC(map_obj, k1, k2) (map_obj->EqualFunc ? map_obj->EqualFunc(k1, k2) : k1 == k2)
#define FREEKEY_FUNC(map_obj, k) (map_obj->FreeKeyFunc ? map_obj->FreeKeyFunc(k) : free(k))

void _sdlex_hashmap_removestash_by_index(CuckooHashMap * map_obj, int index) {
	// If the removed location was not last, move the last tuple to the removed location.
	map_obj->_stashSize--;
	int lastIndex = map_obj->_capacity + map_obj->_stashSize;
	if (map_obj->AutoFreeWhenRemove) {
		FREEKEY_FUNC(map_obj, map_obj->_keyTable[index]);
	}
	if (index < lastIndex) {
		map_obj->_keyTable[index] = map_obj->_keyTable[lastIndex];
		map_obj->_valueTable[index] = map_obj->_valueTable[lastIndex];
		map_obj->_keyTable[lastIndex] = NULL;
		map_obj->_valueTable[lastIndex] = NULL;
	}
	else {
		map_obj->_keyTable[lastIndex] = NULL;
		map_obj->_valueTable[index] = NULL;
	}
}

void * _sdlex_hashmap_removestash(CuckooHashMap * map_obj, void * key) {
	void ** keyTable = map_obj->_keyTable;
	for (int i = map_obj->_capacity, n = i + map_obj->_stashSize; i < n; i++) {
		if (EQUALS_FUNC(map_obj, key, keyTable[i])) {
			void * oldValue = map_obj->_valueTable[i];
			_sdlex_hashmap_removestash_by_index(map_obj, i);
			map_obj->Size--;
			return oldValue;
		}
	}
	return NULL;
}

void * _sdlex_hashmap_getstash(CuckooHashMap * map_obj, void * key, void * defaultValue) {
	for (int i = map_obj->_capacity, n = i + map_obj->_stashSize; i < n; i++)
		if (EQUALS_FUNC(map_obj, key, map_obj->_keyTable[i])) return map_obj->_valueTable[i];
	return defaultValue;
}

void * get_cuckoo_hashmap(CuckooHashMap * map_obj, void * key) {
	int hashCode = map_obj->HashFunc ? map_obj->HashFunc(key) : (int)key;
	int index = hashCode & map_obj->_mask;
	if (!EQUALS_FUNC(map_obj, key, map_obj->_keyTable[index])) {
		index = hash2(map_obj, hashCode);
		if (!EQUALS_FUNC(map_obj, key, map_obj->_keyTable[index])) {
			index = hash3(map_obj, hashCode);
			if (!EQUALS_FUNC(map_obj, key, map_obj->_keyTable[index])) return _sdlex_hashmap_getstash(map_obj, key, NULL);
		}
	}
	return map_obj->_valueTable[index];
}

void * put_cuckoo_hashmap(CuckooHashMap * map_obj, void * key, void * value) {
	void ** keyTable = map_obj->_keyTable;

	// Check for existing keys.
	int hashCode = map_obj->HashFunc ? map_obj->HashFunc(key) : (int)key;
	int index1 = hashCode & map_obj->_mask;
	void * key1 = keyTable[index1];
	if (EQUALS_FUNC(map_obj, key, key1)) {
		void * oldValue = map_obj->_valueTable[index1];
		map_obj->_valueTable[index1] = value;
		if (map_obj->AutoFreeWhenRemove)
			FREEKEY_FUNC(map_obj, key);
		return oldValue;
	}

	int index2 = hash2(map_obj, hashCode);
	void * key2 = keyTable[index2];
	if (EQUALS_FUNC(map_obj, key, key2)) {
		void * oldValue = map_obj->_valueTable[index2];
		map_obj->_valueTable[index2] = value;
		if (map_obj->AutoFreeWhenRemove)
			FREEKEY_FUNC(map_obj, key);
		return oldValue;
	}

	int index3 = hash3(map_obj, hashCode);
	void * key3 = keyTable[index3];
	if (EQUALS_FUNC(map_obj, key, key3)) {
		void * oldValue = map_obj->_valueTable[index3];
		map_obj->_valueTable[index3] = value;
		if (map_obj->AutoFreeWhenRemove)
			FREEKEY_FUNC(map_obj, key);
		return oldValue;
	}

	// Update key in the stash.
	for (int i = map_obj->_capacity, n = i + map_obj->_stashSize; i < n; i++) {
		if (EQUALS_FUNC(map_obj, key, map_obj->_keyTable[i])) {
			void * oldValue = map_obj->_valueTable[i];
			map_obj->_valueTable[i] = value;
			if (map_obj->AutoFreeWhenRemove)
				FREEKEY_FUNC(map_obj, key);
			return oldValue;
		}
	}

	// Check for empty buckets.
	if (key1 == NULL) {
		map_obj->_keyTable[index1] = key;
		map_obj->_valueTable[index1] = value;
		if (map_obj->Size++ >= map_obj->_threshold) _sdlex_hashmap_resize(map_obj, map_obj->_capacity << 1);
		return NULL;
	}

	if (key2 == NULL) {
		map_obj->_keyTable[index2] = key;
		map_obj->_valueTable[index2] = value;
		if (map_obj->Size++ >= map_obj->_threshold) _sdlex_hashmap_resize(map_obj, map_obj->_capacity << 1);
		return NULL;
	}

	if (key3 == NULL) {
		map_obj->_keyTable[index3] = key;
		map_obj->_valueTable[index3] = value;
		if (map_obj->Size++ >= map_obj->_threshold) _sdlex_hashmap_resize(map_obj, map_obj->_capacity << 1);
		return NULL;
	}

	_sdlex_hashmap_push(map_obj, key, value, index1, key1, index2, key2, index3, key3);
	return NULL;
}

void * remove_from_cuckoo_hashmap(CuckooHashMap * map_obj, void * key) {
	int hashCode = map_obj->HashFunc ? map_obj->HashFunc(key) : (int)key;
	int index = hashCode & map_obj->_mask;
	if (EQUALS_FUNC(map_obj, key, map_obj->_keyTable[index])) {
		if (map_obj->AutoFreeWhenRemove) {
			FREEKEY_FUNC(map_obj, map_obj->_keyTable[index]);
		}
		map_obj->_keyTable[index] = NULL;
		void * oldValue = map_obj->_valueTable[index];
		map_obj->_valueTable[index] = NULL;
		map_obj->Size--;
		return oldValue;
	}

	index = hash2(map_obj, hashCode);
	if (EQUALS_FUNC(map_obj, key, map_obj->_keyTable[index])) {
		if (map_obj->AutoFreeWhenRemove) {
			FREEKEY_FUNC(map_obj, map_obj->_keyTable[index]);
		}
		map_obj->_keyTable[index] = NULL;
		void * oldValue = map_obj->_valueTable[index];
		map_obj->_valueTable[index] = NULL;
		map_obj->Size--;
		return oldValue;
	}

	index = hash3(map_obj, hashCode);
	if (EQUALS_FUNC(map_obj, key, map_obj->_keyTable[index])) {
		if (map_obj->AutoFreeWhenRemove) {
			FREEKEY_FUNC(map_obj, map_obj->_keyTable[index]);
		}
		map_obj->_keyTable[index] = NULL;
		void * oldValue = map_obj->_valueTable[index];
		map_obj->_valueTable[index] = NULL;
		map_obj->Size--;
		return oldValue;
	}

	return _sdlex_hashmap_removestash(map_obj, key);
}

void destroy_cuckoo_hashmap(CuckooHashMap * map_obj) {
	if (map_obj->AutoFreeWhenRemove) {
		for (int i = 0, n = map_obj->_capacity + map_obj->_stashCapacity; i < n; i++) {
			FREEKEY_FUNC(map_obj, map_obj->_keyTable[i]);
			free(map_obj->_valueTable[i]);
		}
	}
	free(map_obj->_keyTable);
	free(map_obj->_valueTable);
	free(map_obj);
}

int sdlex_hash_int(void * pt) {
	return *((int *) pt);
}

int sdlex_equal_int(void * pt1, void * pt2) {
	if (pt1 == NULL && pt2 != NULL)
		return 0;
	if (pt1 != NULL && pt2 == NULL)
		return 0;
	return (*((int *)pt1)) == (*((int *)pt2));
}

IntIntCuckooHashMap * create_intint_cuckoo_hashmap() {
	return create_cuckoo_hashmap_p(DEFAULT_SIZE, DEFAULT_LOAD_FACTOR, 1, sdlex_hash_int, sdlex_equal_int, NULL);
}

void put_intint_cuckoo_hashmap(IntIntCuckooHashMap * map_obj, int key, int value) {
	int * pkey = malloc(sizeof(int));
	int * pvalue = malloc(sizeof(int));
	*pkey = key;
	*pvalue = value;
	int * fetched = put_cuckoo_hashmap(map_obj, pkey, pvalue);
	if (fetched) {
		free(fetched);
	}
}

int get_intint_cuckoo_hashmap(IntIntCuckooHashMap * map_obj, int key) {
	int * pkey = malloc(sizeof(int));
	*pkey = key;
	int * fetched = get_cuckoo_hashmap(map_obj, pkey);
	free(pkey);
	return *fetched;
}

int remove_from_intint_cuckoo_hashmap(IntIntCuckooHashMap * map_obj, int key) {
	int * pkey = malloc(sizeof(int));
	*pkey = key;
	int * fetched = remove_from_cuckoo_hashmap(map_obj, pkey);
	free(pkey);
	int result = *fetched;
	free(fetched);
	return result;
}

