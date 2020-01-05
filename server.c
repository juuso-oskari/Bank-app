//
// Created by juuso on 12.11.2019.
//
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <values.h>
#include <string.h>
#include <unistd.h>
#include "queue.h"
#include "miscellanous.h"
#include <stddef.h>
#include "server.h"
#include "account.h"

pthread_rwlock_t  rwlock = PTHREAD_RWLOCK_INITIALIZER;

static int account_exists(struct Bank *bank, char *id){

    // Reader gets the lock, no writing operations possible
    pthread_rwlock_rdlock(&rwlock);

    for (int i = 0; bank->accounts[i] != NULL; i++) {
        if (strcmp(bank->accounts[i]->id, id) == 0) {
            pthread_rwlock_unlock(&rwlock);
            return i;
        }
    }
    // reader lock unlocked, writing possible again
    pthread_rwlock_unlock(&rwlock);
    return -1;
}

int create_account(struct Bank *bank, char *id, float init_balance){

    if(account_exists(bank, id) != -1){
        printf("Account with id %s already exists!\n", id);
        return -1;
    }
    int i = 0;
    pthread_rwlock_rdlock(&rwlock);
    for (i = 0; bank->accounts[i] != NULL; i++);
    pthread_rwlock_unlock(&rwlock);
    struct Account *new_account = malloc(sizeof(struct Account));
    strcpy(new_account->id, id);
    new_account->balance=init_balance;

    // if thread comes here it acquires write lock, so any other attempts to get read/write lock are denied
    pthread_rwlock_wrlock(&rwlock);
    bank->accounts = realloc(bank->accounts, sizeof(struct Account *)*(i + 1) + sizeof(NULL));
    if(bank->accounts == NULL){
        fprintf(stderr, "Failed to reallocate when creating new account!\n");
        return -1;
    }
    bank->accounts[i] = new_account;
    bank->accounts[i + 1] = NULL;
    // writer lock unlocked
    pthread_rwlock_unlock(&rwlock);
    printf("New account with id: %s and initial balance: %.2f created!\n", id, init_balance);
    return 1;
}

static int print_account_balance(struct Bank *bank, char *id, char *response_buffer) {

    pthread_rwlock_rdlock(&rwlock);
    int account_index = account_exists(bank, id);
    if(account_index == -1) {
        printf("No such account created!\n");
        snprintf(response_buffer, 100, "No account with id %s created!\n", id);
        pthread_rwlock_unlock(&rwlock);
        return 0;
    }
    printf("Account id: %s, balance: %f\n", bank->accounts[account_index]->id, bank->accounts[account_index]->balance);
    snprintf(response_buffer, 100, "Account id: %s, balance: %f\n", bank->accounts[account_index]->id, bank->accounts[account_index]->balance);
    pthread_rwlock_unlock(&rwlock);
    return 1;


}

int process(char *request, struct Bank *bank, char *response, pthread_barrier_t *barrier){
    // initializing everything as empty
    char cmd[30] = "\0";
    char arg[30] = "\0";
    char ac1[30] = "\0";
    char ac2[30] = "\0";
    char sum[30] = "\0";
    strcpy(response, "\0");  // to make response empty initially
    sscanf(request, "%s %[^\n]", cmd, arg);
    printf("Command part: %s, argument part: %s\n", cmd, arg);
    if(strcmp(cmd, "l")==0){
        if(sscanf(arg, "%s", ac1) == 1){
            print_account_balance(bank, ac1, response);
        }else{
            printf("Wrong number of arguments!\n");
            return 0;
        }
    } else if(strcmp(cmd, "t")==0){
        if(sscanf(arg, "%s %s %s", ac1, ac2, sum) == 3){
            int ac1_index = account_exists(bank, ac1);
            int ac2_index = account_exists(bank, ac2);
            if(ac1_index == -1 || ac2_index == -1){
                snprintf(response, 100, "Account id's are incorrect or don't exist.\n");
                return 0;
            }
            float amount = withdraw(bank->accounts[ac1_index], strtof(sum, NULL));
            if(deposit(bank->accounts[ac2_index], amount)==1){
                snprintf(response, 100, "Managed to transfer %f from account %s to account %s\n", amount, ac1, ac2);
            }
        }
    } else if(strcmp(cmd, "w")==0){
        if(sscanf(arg, "%s %s", ac1, sum) == 2) {
            int ac1_index = account_exists(bank, ac1);
            if(ac1_index == -1) {snprintf(response, 100, "Account id is incorrect or doesn't exist.\n"); return 0;}
            float withdrew = withdraw(bank->accounts[ac1_index], strtof(sum, NULL));
            if(withdrew > 0){
                snprintf(response, 100, "Managed to withdraw %f from account %s\n", withdrew, ac1);
            } else{
                snprintf(response, 100, "Cannot withdraw negative sum, try deposit\n");
            }


        } else{
            printf("Wrong number of arguments!\n");
            snprintf(response, 100, "Wrong number of arguments\n");
            return 0;
        }
    } else if(strcmp(cmd, "d")==0){
        if(sscanf(arg, "%s %s", ac1, sum) == 2) {
            int ac1_index = account_exists(bank, ac1);
            if(ac1_index == -1) {snprintf(response, 100, "Account id is incorrect or doesn't exist.\n"); return 0;}
            if(deposit(bank->accounts[ac1_index], strtof(sum, NULL)) == 1){
                snprintf(response, 100, "Managed to deposit %s to account %s\n", sum, ac1);
            } else{
                snprintf(response, 100, "Cannot deposit negative sum, try withdraw\n");
            }
        } else{
            printf("Wrong number of arguments with!\n");
            snprintf(response, 100, "Wrong number of arguments!\n");
             return 0;
        }
    } else if(strcmp(cmd, "c")==0){
        int arg_num = sscanf(arg, "%s %s", ac1, sum);
        if(arg_num == 2){
            if(strtof(sum, NULL) < 0){
                snprintf(response, 100, "Cannot initiate account with negative balance!\n");
            }
            create_account(bank, ac1, strtof(sum, NULL));
        } else if (arg_num == 1){
            create_account(bank, ac1, 0);
        } else{
            printf("Wrong number of arguments!\n");
            return 0;
        }
    } else if(strcmp(cmd, "b")==0){
        pthread_barrier_wait(barrier);
        return 1;
    } else{
        printf("Unknown command encountered: %s\n", request);
        snprintf(response, 100, "Unknown command encountered: %s\n", request);
    }

    return 1;
}
