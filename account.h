//
// Created by juuso on 9.11.2019.
//

#ifndef ASSIGNMENT2_ACCOUNT_H
#define ASSIGNMENT2_ACCOUNT_H
#define MAX_ID 50
struct Account {
    char id[MAX_ID];
    float balance;
};

int deposit(struct Account *a, float amount);  // returns 1 for success, 0 for failure

float withdraw(struct Account *a, float amount);  // returns the amount successfully withdrawn

#endif //ASSIGNMENT2_ACCOUNT_H
