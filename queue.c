//
// Created by juuso on 7.11.2019.
//
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "queue.h"

void list_requests(struct Queue *q){
    while(q->next!=NULL){
        printf("%s\n", (q->next)->request);
        q=q->next;
    }
}

int isEmpty(struct Queue *q){
    if(q->next==NULL){
        return 1;
    }else{
        return 0;
    }
}
int overtake(struct Queue *oq, const char *request){
    if(!isEmpty(oq)) {
        struct Queue *new = malloc(sizeof(struct Queue));
        new->request = malloc(strlen(request) + 1);
        strcpy(new->request, request);
        new->next = oq->next;
        oq->next = new;
        oq->size +=1;
    }else{
        enqueue(oq, request);
    }
    return 1;
}


int enqueue(struct Queue *oq, const char *request){
    struct Queue *q = oq;
    while(q->next!=NULL){
        q=q->next;
    }

    struct Queue *new = malloc(sizeof(struct Queue));
    new->request=malloc(strlen(request) + 1);

    strcpy(new->request, request);
    if(new->request == NULL){
        return 0;
    }

    new->next=NULL;
    q->next=new;
    oq->size += 1;
    return 1;
}

int dequeue(struct Queue *q, char *buffer){

    if(q->next!=NULL){
        strcpy(buffer, q->next->request);
        struct Queue *temp = q->next;
        q->next=q->next->next;
        free(temp->request);
        free(temp);
        q->size -=1;
        return 1;
    } else{
        return 0;
    }


}