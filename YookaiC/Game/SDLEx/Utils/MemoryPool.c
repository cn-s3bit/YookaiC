#include "MemoryPool.h"
#include "Deque.h"

Deque * deque_4_bytes = NULL;

void * memorypool_malloc(size_t size) {
	if (size != 4) {
		return malloc(size);
	}
	if (deque_4_bytes == NULL) {
		deque_4_bytes = create_deque(4, 16);
		return malloc(4);
	}
	void * pt;
	if (poll_deque_head(deque_4_bytes, &pt) != 0)
		return malloc(4);
	return pt;
}

void memorypool_free(void * pt, size_t size) {
	if (size != 4) {
		free(pt);
		return;
	}
	if (pt == NULL)
		return;
	if (deque_4_bytes == NULL)
		deque_4_bytes = create_deque(4, 16);
	push_deque_head(deque_4_bytes, &pt);
}

void memorypool_free_4bytes(void * pt) {
	if (pt == NULL)
		return;
	if (deque_4_bytes == NULL)
		deque_4_bytes = create_deque(4, 16);
	push_deque_head(deque_4_bytes, &pt);
}
