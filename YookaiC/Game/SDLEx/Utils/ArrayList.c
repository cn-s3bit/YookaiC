#include "ArrayList.h"
#include <string.h>
#include <stdlib.h>

ArrayList * create_array_list(unsigned elementSize, unsigned initialCapacity) {
	ArrayList * list_obj = malloc(sizeof(ArrayList));
	list_obj->Size = 0;
	list_obj->_capacity = initialCapacity;
	list_obj->_data = malloc(elementSize * initialCapacity);
	list_obj->ElementSize = elementSize;
	return list_obj;
}

void destroy_array_list(ArrayList * arraylist) {
	free(arraylist->_data);
	free(arraylist);
}

int get_element_from_array_list(ArrayList * arraylist, unsigned index, void * outItem) {
	if (arraylist->Size <= index) {
		return ARRAYLIST_INDEX_OUT_OF_RANGE;
	}
	memcpy(outItem, ((char *)arraylist->_data) + index * arraylist->ElementSize, arraylist->ElementSize);
	return 0;
}

int assign_array_list_element(ArrayList * arraylist, unsigned index, const void * inItem) {
	if (arraylist->Size <= index) {
		return ARRAYLIST_INDEX_OUT_OF_RANGE;
	}
	memcpy(((char *)arraylist->_data) + index * arraylist->ElementSize, inItem, arraylist->ElementSize);
	return 0;
}

void append_array_list(ArrayList * arraylist, void * inItem) {
	if (arraylist->Size == arraylist->_capacity) {
		void * oldData = arraylist->_data;
		size_t oldsize = arraylist->_capacity * arraylist->ElementSize;
		arraylist->_data = malloc(oldsize * 2);
		arraylist->_capacity *= 2;
		memcpy(arraylist->_data, oldData, oldsize);
		free(oldData);
	}
	memcpy(((char *)arraylist->_data) + arraylist->Size * arraylist->ElementSize, inItem, arraylist->ElementSize);
	++arraylist->Size;
}
