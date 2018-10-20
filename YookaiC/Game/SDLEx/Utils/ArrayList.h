#ifndef SDLEX_ARRAYLIST_H
#define SDLEX_ARRAYLIST_H
#define ARRAYLIST_INDEX_OUT_OF_RANGE 1
typedef struct ArrayList {
	void * _data;
	unsigned ElementSize;
	unsigned Size;
	unsigned _capacity;
} ArrayList;

ArrayList * create_array_list(unsigned elementSize, unsigned initialCapacity);
void destroy_array_list(ArrayList * arraylist);
int get_element_from_array_list(ArrayList * arraylist, unsigned index, void * outItem);
int assign_array_list_element(ArrayList * arraylist, unsigned index, const void * inItem);
void append_array_list(ArrayList * arraylist, void * inItem);
#endif
