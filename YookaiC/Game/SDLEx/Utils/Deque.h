#ifndef SDLEX_DEQUE_H
#define SDLEX_DEQUE_H
#include "ArrayList.h"
#define DEQUEUE_NO_ITEM 2
typedef struct Deque {
	ArrayList _arrayList;
	int _head, _tail;
} Deque;

Deque * create_deque(unsigned elementSize, unsigned initialCapacity);
void destroy_deque(Deque * deque);
int push_deque_head(Deque * deque, const void * inItem);
int poll_deque_head(Deque * deque, void * outItem);
int peek_deque_head(Deque * deque, void * outItem);
int push_deque_tail(Deque * deque, const void * inItem);
int poll_deque_tail(Deque * deque, void * outItem);
int peek_deque_tail(Deque * deque, void * outItem);
int get_deque_size(Deque * deque);
int assign_deque_element(Deque * deque, unsigned index_from_head, const void * inItem);
int get_deque_element(Deque * deque, unsigned index_from_head, void * outItem);
#endif
