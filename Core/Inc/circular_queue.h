#ifndef CIRCULAR_QUEUE_DEFINED
#define CIRCULAR_QUEUE_DEFINED

#include "rust.h"

struct circular_queue {
    u64 *buf;
    u64 len;
    u64 head;
    u64 size;
};

struct circular_queue circular_queue_new(u64 len);
void circular_queue_push_back(struct circular_queue *q, u64 value);
u64 circular_queue_get_back(struct circular_queue *q, u64 idx);

#endif  // CIRCULAR_QUEUE_DEFINED