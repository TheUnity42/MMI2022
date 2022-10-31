#include "adc_queue.h"
#include <stdlib.h>
#include <string.h>

void queue_initialize(Queue *queue, uint16_t max_size) {
	queue->max_size = max_size;
	queue->size = 0;
	queue->head = NULL;
	queue->tail = NULL;
}

uint8_t queue_is_empty(Queue *queue) { return queue->size == 0 || queue->tail == NULL; }

uint8_t queue_is_full(Queue *queue) { return queue->size == queue->max_size; }

uint8_t queue_enqueue(Queue *queue, Sample data) {
	// check if the queue is full
	if (queue_is_full(queue)) {
		return queue->size;
	}

	// create a new node
	struct QUEUE_NODE_T *node = (struct QUEUE_NODE_T *)malloc(sizeof(struct QUEUE_NODE_T));
	node->data = data;
	node->next = NULL;

	// if the queue has a tail, add the new node to the end
	if (queue->tail) {
		queue->tail->next = node;
		queue->tail = node;
	} else {
		// otherwise, set the head and tail to the new node
		queue->head = node;
		queue->tail = node;
	}
	// increment the size
	queue->size++;
	// return success
	return 0;
}

uint8_t queue_dequeue(Queue *queue, Sample *data) {
	// check if the queue is empty
	if (queue_is_empty(queue)) {
		return 1;
	}

	// get the head node
	struct QUEUE_NODE_T *node = queue->head;
	// get the data
	memcpy(data, &node->data, sizeof(Sample));
	// set the head to the next node
	queue->head = node->next;
	// if the head is null, the queue is empty
	if (!queue->head) {
		queue->tail = NULL;
	}
	// decrement the size
	queue->size--;
	// free the node
	free(node);

	// return success  
	return 0;
}