//
// Created by juuso on 9.11.2019.
//

#include "account.h"


int deposit(struct Account *a, float amount){
    if(amount >= 0){
        a->balance += amount;
        return 1;
    }else{
        return -1;
    }


}

float withdraw(struct Account *a, float amount){
    if(amount < 0){
        return -1;
    }
    if(a->balance>=amount){
        a->balance-=amount;
        return amount;
    } else{
        float ret = a->balance;
        a->balance = 0;
        return ret;
    }
}