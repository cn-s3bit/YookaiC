#include "MemoryPool.h"
#include "Deque.h"

Deque * deque_4_bytes = NULL;

void * memorypool_malloc(size_t size) {
	if (size != 4) {
		return malloc(size);
	}
	if (deque_4_bytes == NULL)
		deque_4_bytes = create_deque(4, 16);
	if (get_deque_size(deque_4_bytes) == 0)
		return malloc(4);
	void * pt;
	poll_deque_head(deque_4_bytes, &pt);
	return pt;
}

void memorypool_free(void * pt, size_t size) {
	if (size != 4) {
		free(pt);
		return;
	}
	if (deque_4_bytes == NULL)
		deque_4_bytes = create_deque(4, 16);
	if (pt == NULL)
		return;
	push_deque_head(deque_4_bytes, &pt);
}

void memorypool_free_4bytes(void * pt) {
	if (deque_4_bytes == NULL)
		deque_4_bytes = create_deque(4, 16);
	if (pt == NULL)
		return;
	push_deque_head(deque_4_bytes, &pt);
}
