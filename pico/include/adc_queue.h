#ifndef ADC_QUEUE_H
#define ADC_QUEUE_H

#include "pico/stdlib.h"

typedef struct SAMPLE_T {
    uint16_t value;
    uint64_t timestamp;
} Sample;

struct QUEUE_NODE_T {
	Sample data;
	struct QUEUE_NODE_T *next;
};

typedef struct QUEUE_T {
    struct QUEUE_NODE_T *head;
    struct QUEUE_NODE_T *tail;
    uint16_t size;
    uint16_t max_size;
} Queue;

void queue_initialize(Queue *queue, uint16_t max_size);

uint8_t queue_is_empty(Queue *queue);

uint8_t queue_is_full(Queue *queue);

uint8_t queue_enqueue(Queue *queue, Sample data);

uint8_t queue_dequeue(Queue *queue, Sample* data);

#endif /* ADC_QUEUE_H */