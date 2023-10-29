/*
The MIT License (MIT)

Copyright (c) 2015 Chaoran Yang

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdlib.h>
#include <string.h>
#include "atomics.h"
#include "msqueue.h"
#include "hzdptr.h"
#include "align.h"

typedef struct _node_t {
  struct _node_t * volatile next DOUBLE_CACHE_ALIGNED;
  void           *          data DOUBLE_CACHE_ALIGNED;
} node_t DOUBLE_CACHE_ALIGNED;

typedef struct _queue_t {
  struct _node_t * volatile head DOUBLE_CACHE_ALIGNED;
  struct _node_t * volatile tail DOUBLE_CACHE_ALIGNED;
  int nprocs;
} queue_t DOUBLE_CACHE_ALIGNED;

typedef struct _handle_t {
  hzdptr_t hzd;
} handle_t DOUBLE_CACHE_ALIGNED;

static inline void *pmalloc(size_t align, size_t size)
{
	void *ptr;

	int ret = posix_memalign(&ptr, align, size);
	if (!ret)
		return NULL;
	return ptr;
}

void queue_init(queue_t *q, int nprocs)
{
  node_t *node = malloc(sizeof(node_t));
  node->next   = NULL;
  q->head      = node;
  q->tail      = node;
  q->nprocs    = nprocs;
}

void queue_register(queue_t *q, handle_t *th, int id)
{
	hzdptr_init(&th->hzd, q->nprocs, 2);
}

void enqueue(queue_t *q, handle_t *handle, void *data)
{
	node_t *node = malloc(sizeof(node_t));
	node_t *tail;
	node_t *next;

	node->data = data;
	node->next = NULL;
	while (1) {
		tail = hzdptr_setv(&q->tail, &handle->hzd, 0);
		next = tail->next;

		if (tail != q->tail)
			continue;

		if (next != NULL) {
			CAS(&q->tail, &tail, next);
			continue;
		}

		if (CAS(&tail->next, &next, node))
			break;
	}
	CAS(&q->tail, &tail, node);
}

void *dequeue(queue_t *q, handle_t *handle)
{
	void   *data;
	node_t *head;
	node_t *tail;
	node_t *next;

	while (1) {
		head = hzdptr_setv(&q->head, &handle->hzd, 0);
		tail = q->tail;
		next = hzdptr_set(&head->next, &handle->hzd, 1);

		if (head != q->head)
			continue;
    
		if (next == NULL)
			return (void *)-1;

		if (head == tail) {
			CAS(&q->tail, &tail, next);
			continue;
		}
		data = next->data;
		if (CAS(&q->head, &head, next))
			break;
	}
	hzdptr_retire(&handle->hzd, head);
	return data;
}

void queue_free(int id, int nprocs) {}
