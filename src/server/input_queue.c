#include "input_queue.h"

void input_queue_init(input_queue* q)
{
    q->start = 0;
    q->size = 0;
}

void input_queue_put(input_queue* q, direction dir)
{
    if (q->size >= INPUT_QUEUE_SIZE)
        return;

    size_t end = (q->start + q->size) % INPUT_QUEUE_SIZE;
    q->arr[end] = dir;
    q->size++;
}

bool input_queue_get(input_queue* q, direction* dir)
{
    if (q->size == 0)
        return false;

    *dir = q->arr[q->start];
    q->start = (q->start + 1) % INPUT_QUEUE_SIZE;
    q->size--;
    return true;
}
