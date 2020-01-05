// Server-side functions
// Created by juuso on 12.11.2019.
//
#include "queue.h"
#include "bank.h"

#ifndef ASSIGNMENT2_SERVER_H
#define ASSIGNMENT2_SERVER_H
int create_account(struct Bank *bank, char *id, float init_balance);
int process(char *request, struct Bank *bank, char *response, pthread_barrier_t *our_barrier);
#endif //ASSIGNMENT2_SERVER_H
