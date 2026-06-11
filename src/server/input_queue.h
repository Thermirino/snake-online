#ifndef INPUT_QUEUE_H
#define INPUT_QUEUE_H

#include <game.h>

#define INPUT_QUEUE_SIZE    3

typedef struct {
    direction arr[INPUT_QUEUE_SIZE];
    size_t start;
    size_t size;
} input_queue;

void input_queue_init(input_queue* q);
void input_queue_put(input_queue* q, direction dir);
bool input_queue_get(input_queue* q, direction* dir);

#endif
