//
// Created by juuso on 7.11.2019.
//

#ifndef ASSIGNMENT2_QUEUE_H
#define ASSIGNMENT2_QUEUE_H

struct Queue {
    char *request;  // server-request (dynamically allocated)
    struct Queue *next;  // (pointer to next request in queue)
    int size;
};

void list_requests(struct Queue *q);

int overtake(struct Queue *q, const char *request);

int isEmpty(struct Queue *q);

int enqueue(struct Queue *q, const char *request);

int dequeue(struct Queue *q, char *buffer);
#endif //ASSIGNMENT2_QUEUE_H
