//
// Created by juuso on 14.11.2019.
//

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <unistd.h>

#include <pthread.h>
#include <values.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "queue.h"
#include "miscellanous.h"
#include "server.h"
#include "account.h"


#define NUM_OF_CLIENTS 1
#define PORT_NUM 9005


int NUM_OF_SERVERS = 4;

struct arg_struct {
    int id;
    int csock;
};

// Getting the mutex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_barrier_t barrier;



// Global variables
int keepRunning = 1;
struct Queue **server_queues;  // array of server queues
struct Bank bank = {.balance = 0, NULL};

void *server_function(void *arg);  // prototype, implementation in the end


int main(int argc,char* argv[]){

    if(argc == 2){
        char *e;
        if(strtol(argv[1], &e, 10) != 0){
            NUM_OF_SERVERS = strtol(argv[1], &e, 10);
        }

    }
    
    printf("\n\nThis terminal is for server side outputs.\n\n");
    fflush(stdout);
    char logtext[100];  // for parsed log messages
    wlog("Starting the program.");
    FILE *f = fopen("logfile.log", "w"); // cleans log
    fclose(f);
    // initializations
    pthread_t *thread_group = malloc(sizeof(pthread_t) * NUM_OF_SERVERS);
    server_queues = malloc(sizeof(struct Queue*) * NUM_OF_SERVERS + sizeof(NULL));
    server_queues[NUM_OF_SERVERS] = NULL;
    // last member of the banks account pointer array is NULL pointer
    bank.accounts = malloc(sizeof(NULL));
    *(bank.accounts)= NULL;
    pthread_barrier_init(&barrier, NULL, NUM_OF_SERVERS + 1);
    // Following is socket programming
    char server_message[256] = "You've reached the bank server!\n\n";
    // create a server socket
    wlog("Creating TCP/IP-socket.");
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0)
        perror("Error opening the server socket\n");
    // to be able to reconnect quickly,
    int enable = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        printf("setsockopt(SO_REUSEADDR) failed\n");
    //define server address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT_NUM);  // same port as with client
    server_address.sin_addr.s_addr = INADDR_ANY;

    // bind the socket to our specified IP and port
    bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));
    // listen to the client
    listen(server_socket, NUM_OF_CLIENTS);
    // to save the id of the client socket for future data sharing
    int client_socket;
    wlog("Waiting for client to join.");
    client_socket = accept(server_socket, NULL, NULL);  // waits for connection of client
    if(client_socket == -1){
        exit(errno);
    }
    wlog("Client connecting...");
    // creating threads
    for(int i = 0; i < NUM_OF_SERVERS; i++){
        struct arg_struct *args = calloc(1, sizeof(struct arg_struct));  // thread arguments
        args->id = i;
        args->csock = client_socket;
        pthread_create(&thread_group[i], NULL, server_function, (void *)args);
        usleep(1000*200);  // just for visualisation
    }
    snprintf(logtext, sizeof(logtext), "Created %d server threads.", NUM_OF_SERVERS);
    wlog(logtext);
    // sending the server_message as a response to successful connection of client
    send(client_socket, server_message, sizeof(server_message), 0);
    wlog("Client joined! Now waiting for requests from client.");
    // communication between client and the server
    char input[500];
    while(keepRunning) {
        // receive requests from the client as input
        strcpy(input, "\0");
        int n = 1;  // for the case that client exits by ctrl + c
        n = recv(client_socket, input, sizeof(input) - 1, 0);
        if(n == 0){
            printf("Client exited with ctrl + c command!\n");
            break;
        } else if(n < 0){
            fprintf(stderr, "Client exited with error number: %d\n", errno);
        }
        input[n] = '\0';
        printf("Received request %s from client.\n", input);
        snprintf(logtext, sizeof(logtext), "Received request %s from client.", input);
        wlog(logtext);
        if(strcmp(input,"e") == 0) {
            printf("\n\nClient exited the bank!\n\n");
            wlog("Client exited the bank!");
            keepRunning = 0;  //this also ends threads

        }else if(strcmp(input,"b") == 0){
            wlog("Client initiated query for bank balance.");
            for (int i = 0; i < NUM_OF_SERVERS; i++){
                overtake(server_queues[i], input);  // places balance query as first pending request to all servers
            }
            wlog("Waiting for servers to reply to master query!\n");
            pthread_barrier_wait(&barrier);
            wlog("All threads answered to master query!\n");
            float bank_balance = 0;
            for (int i = 0; bank.accounts[i] != NULL; i++) {
                bank_balance += bank.accounts[i]->balance;
            }
            snprintf(server_message, sizeof(server_message), "Bank balance: %f\n", bank_balance);
            printf("Bank balance: %f\n", bank_balance);
            send(client_socket, server_message, sizeof(server_message), 0);
        }else{
            int min_length; int min_index;
            min(server_queues, &min_length, &min_index);
            pthread_mutex_lock(&mutex);
            snprintf(logtext, sizeof(logtext), "Directing request to server %d.", min_index);
            wlog(logtext);
            if(enqueue(server_queues[min_index], input) == 0){
                fprintf(stderr, "Error with directing the request %s to server!\n", input);
                send(client_socket, server_message, sizeof(server_message), 0);
            }
            pthread_mutex_unlock(&mutex);
        }
    }
    //closing the socket
    wlog("Closing the TCP/IP-socket.");
    close(server_socket);
    // free's
    // wait for all threads to finish
    for(int i = 0; i < NUM_OF_SERVERS; i++) {
        pthread_join(thread_group[i], NULL);
    }
    free(thread_group);
    for (int ind = 0; bank.accounts[ind] != NULL; ind++){
        free(bank.accounts[ind]);
    }
    free(bank.accounts);
    for (int ind = 0; ind > NUM_OF_SERVERS; ind++){
        char holder_buffer[100];
        while(dequeue(server_queues[ind], holder_buffer));
        free(server_queues[ind]);
    }
    free(server_queues);

    return 0;
}

void *server_function(void *arg){

    struct arg_struct *args = arg;
    int my_id = args->id;
    int client_socket = args->csock;
    printf("Server with id: %d, and client sock: %d started\n", my_id, client_socket);

    static char empty[] = "\0";
    struct Queue my_queue = {empty, NULL, 0};  // in my queue implementation first member is empty

    pthread_mutex_lock(&mutex);
    server_queues[my_id] = &my_queue;
    pthread_mutex_unlock(&mutex);

    char request[256];
    char logtext[100];

    while(keepRunning){

        if(!isEmpty(&my_queue)){
            if(dequeue(&my_queue, request) != 1) {
                fprintf(stderr, "Server %d having error handling request %s!\n", my_id, request);
            } else {
                // do processing
                snprintf(logtext, sizeof(logtext), "Server %d starting to process request %s.", my_id, request);
                wlog(logtext);
                char response[100] = "\0";
                process(request, &bank, response, &barrier);
                wlog(response);
                send(client_socket, response, sizeof(response), 0);
            }
        }
    }
    free(args);
    pthread_exit(0);
}

