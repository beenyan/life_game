
#include "circular_queue.h"

#include <stdlib.h>

#include "rust.h"

struct circular_queue circular_queue_new(u64 len) {
    struct circular_queue q;
    q.buf = (u64 *)malloc(len * sizeof(u64));
    q.len = len;
    q.head = 0;
    q.size = 0;

    return q;
}

void circular_queue_push_back(struct circular_queue *q, u64 value) {
    q->buf[q->head] = value;
    q->head = (q->head + 1) % q->len;
    if (q->size < q->len) {
        q->size++;
    }
}

u64 circular_queue_get_back(struct circular_queue *q, u64 idx) {
    if (idx > q->size) {
        exit(EXIT_FAILURE);
    }

    i64 head = q->head - idx - 1;
    while (head < 0) {
        head += q->len;
    }

    return q->buf[head];
}
