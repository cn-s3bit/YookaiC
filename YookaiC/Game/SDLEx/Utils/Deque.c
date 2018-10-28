#include "Deque.h"
#include <stdlib.h>
#include <string.h>

Deque * create_deque(unsigned elementSize, unsigned initialCapacity) {
	Deque * deque_obj = malloc(sizeof(Deque));
	deque_obj->_arrayList.Size = 0;
	deque_obj->_arrayList._capacity = initialCapacity;
	deque_obj->_arrayList._data = malloc(elementSize * initialCapacity);
	deque_obj->_arrayList.ElementSize = elementSize;
	deque_obj->_head = deque_obj->_tail = initialCapacity / 2;
	void * tmp_trash = malloc(elementSize);
	for (unsigned i = 0; i < initialCapacity; i++) {
		append_array_list((ArrayList *)deque_obj, tmp_trash);
	}
	free(tmp_trash);
	return deque_obj;
}
void destroy_deque(Deque * deque) {
	free(deque->_arrayList._data);
	free(deque);
}
int push_deque_tail(Deque * deque, const void * inItem) {
	++deque->_tail;
	if (deque->_tail >= (int)deque->_arrayList.Size) {
		deque->_tail -= deque->_arrayList.Size;
	}
	if (deque->_tail == deque->_head) {
		// expand
		unsigned expand = deque->_arrayList.Size;
		void * tmp_trash = malloc(deque->_arrayList.ElementSize);
		for (unsigned i = 0; i < expand; i++) {
			append_array_list((ArrayList *)deque, tmp_trash);
		}
		memcpy(
			((char *)deque->_arrayList._data) + deque->_arrayList.ElementSize * expand,
			((char *)deque->_arrayList._data),
			deque->_arrayList.ElementSize * expand
		);
		deque->_tail += expand;
		free(tmp_trash);
	}
	return assign_array_list_element((ArrayList *)deque, deque->_tail - 1 < 0 ? deque->_tail - 1 + deque->_arrayList.Size : deque->_tail - 1, inItem);
}
int poll_deque_head(Deque * deque, void * outItem) {
	if (deque->_head == deque->_tail)
		return DEQUEUE_NO_ITEM;
	int ret = get_element_from_array_list((ArrayList *)deque, deque->_head, outItem);
	if (ret != 0)
		return ret;
	++deque->_head;
	if (deque->_head >= (int)deque->_arrayList.Size) {
		deque->_head -= deque->_arrayList.Size;
	}
	return 0;
}
int peek_deque_head(Deque * deque, void * outItem) {
	if (deque->_head == deque->_tail)
		return DEQUEUE_NO_ITEM;
	return get_element_from_array_list((ArrayList *)deque, deque->_head, outItem);
}
int push_deque_head(Deque * deque, const void * inItem) {
	--deque->_head;
	if (deque->_head < 0) {
		deque->_head += deque->_arrayList.Size;
	}
	if (deque->_tail == deque->_head) {
		// expand
		unsigned expand = deque->_arrayList.Size;
		void * tmp_trash = malloc(deque->_arrayList.ElementSize);
		for (unsigned i = 0; i < expand; i++) {
			append_array_list((ArrayList *)deque, tmp_trash);
		}
		memcpy(
			((char *)deque->_arrayList._data) + deque->_arrayList.ElementSize * expand,
			((char *)deque->_arrayList._data),
			deque->_arrayList.ElementSize * expand
		);
		deque->_tail += expand;
		free(tmp_trash);
	}
	return assign_array_list_element((ArrayList *)deque, deque->_head, inItem);
}
int poll_deque_tail(Deque * deque, void * outItem) {
	if (deque->_head == deque->_tail)
		return DEQUEUE_NO_ITEM;
	--deque->_tail;
	if (deque->_tail < 0) {
		deque->_tail += deque->_arrayList.Size;
	}
	return get_element_from_array_list((ArrayList *)deque, deque->_tail, outItem);
}
int peek_deque_tail(Deque * deque, void * outItem) {
	if (deque->_head == deque->_tail)
		return DEQUEUE_NO_ITEM;
	return get_element_from_array_list((ArrayList *)deque, deque->_tail - 1 < 0 ? deque->_tail - 1 + deque->_arrayList.Size : deque->_tail - 1, outItem);
}
int assign_deque_element(Deque * deque, unsigned index_from_head, const void * inItem) {
	if (deque->_head == deque->_tail)
		return DEQUEUE_NO_ITEM;
	unsigned real_index = deque->_head + index_from_head % get_deque_size(deque);
	return assign_array_list_element((ArrayList *)deque, real_index, inItem);
}
int get_deque_element(Deque * deque, unsigned index_from_head, void * outItem) {
	if (deque->_head == deque->_tail)
		return DEQUEUE_NO_ITEM;
	unsigned real_index = deque->_head + index_from_head % get_deque_size(deque);
	return get_element_from_array_list((ArrayList *)deque, real_index, outItem);
}
int get_deque_size(Deque * deque) {
	int tail = deque->_tail;
	int head = deque->_head;
	if (tail >= head)
		return tail - head;
	else {
		tail += deque->_arrayList.Size;
		return tail - head;
	}
}
