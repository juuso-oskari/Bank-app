//
// Created by juuso on 14.11.2019.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <values.h>
#include <fcntl.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>
#include <setjmp.h>

#define USER_INPUT_LENGTH 30
#define SERVER_IP_ADDRESS "127.0.0.1"  // insert remote server IP-address here, "127.0.0.1" is for localhost
#define IP_PORT_NUM 9005 // high number just to make sure it's open



void *receiver_function(void *arg);
int keepRunning;

jmp_buf return_here;

void sigint_handler(int sig)  //Defines that program will end by signal given as a parameter
{
    longjmp (return_here, 1);
}

int main(int argc,char* argv[]){

    signal(SIGINT, sigint_handler);
    printf("\n\nWelcome to my bank-app prototype!\nThose two extra terminals that opened are for responses to your requests and for server side info.\n\n");
    // to get current working directory
    char cwd[PATH_MAX];

    char filename[20] = "server_executable";

    if(argc == 2){
        strcat(filename, " ");
        strcat(filename, argv[1]);
    }


    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        // printf("Current working dir: %s\n", cwd);
    } else {
        perror("getcwd() error");
        exit(1);
    }
    char filepath[200];
    snprintf(filepath, sizeof(filepath), "%s/%s", cwd, filename);
    char system_call[200];
    snprintf(system_call, sizeof(system_call), "x-terminal-emulator -e \"%s\"", filepath);
    // executes server side program on another terminal
    system(system_call);
    // create a socket
    int network_socket;
    network_socket = socket(AF_INET, SOCK_STREAM, 0);

    // specify an address for the socket
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(IP_PORT_NUM);

    struct in_addr addr;
    //Convert the IP dotted quad into struct in_addr
    inet_aton(SERVER_IP_ADDRESS, &(addr));
    server_address.sin_addr.s_addr = addr.s_addr;

    printf("Connecting to server...\n");
    int connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    if(connection_status == -1){
        perror("Error connecting to remote socket! \n\n");
        return 1;
    }
    // receive connection message from the server
    char server_response[256];
    recv(network_socket, &server_response, sizeof(server_response), 0);
    printf("Got server response: %s\n", server_response);
    keepRunning = 1;
    char *user_input = calloc(USER_INPUT_LENGTH + 2, sizeof(char));
    user_input[0] = '\0';
    // creates a thread to receive server responses
    pthread_t *thread = malloc(sizeof(pthread_t));
    pthread_create(thread, NULL, receiver_function, (void *)&network_socket);

    if (setjmp (return_here) != 0) { //jumps here if SIGINT signal is received
        printf("\nCtrl + c command detected!\nExiting...\n");
        char end[2]="e";
        send(network_socket, end , sizeof(end), 0); // sends command to close server
        keepRunning = 0;  // while loop is never ran
    }

    while (keepRunning) {
        int check = 1;
        while(check) {
            printf("\nGive serve-request (submit with enter, h for help, e to exit):\n");

            if (fgets(user_input, USER_INPUT_LENGTH, stdin)==NULL){
                perror("fgets returned NULL");
            } else{
                char *p;
                p = strchr(user_input, '\n');
                if(p){//check exist newline
                    check = 0;
                    *p = 0;
                } else {
                    printf("Error, input too long\n");
                    scanf("%*[^\n]");scanf("%*c");//clear upto newline
                }
            }
        }
        char delim[] = "\n";
        user_input = strsep(&user_input, delim);
        if (strcmp(user_input, "e") == 0) {
            printf("\n\nExiting the bank!\n\n");
            send(network_socket, user_input, sizeof(user_input), 0);
            keepRunning = 0;
        } else if(strcmp(user_input, "h") == 0){
            printf("Available commands:\n\nb : bank balance\n\nc <id> <initial_balance> : create account with id and initial_balance\n\n"
                   "l <id> : returns account information\n\nw <id> <sum> : withdraws sum from account id\n\nd <id> <sum> : deposits sum to account id\n\n"
                   "t <id_1> <id_2> <sum> : transfer sum from account id_1 to account id_2\n\n\n");
        }else{
            printf("Sending the request: %s to server\n", user_input);
            send(network_socket, user_input, sizeof(char)*USER_INPUT_LENGTH, 0);
        }
    }
    pthread_join(*thread, NULL);
    close(network_socket);
    free(user_input);
    free(thread);

    return 0;
}

void *receiver_function(void *arg){

    int network_socket = *(int *) arg;
    char server_response[256];

    char cwd[PATH_MAX];
    char filename[] = "request_responses.txt";
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        // printf("Current working dir: %s\n", cwd);
    } else {
        perror("getcwd() error");
        exit(1);
    }
    char filepath[200];

    snprintf(filepath, sizeof(filepath), "%s/%s", cwd, filename);

    FILE *f = fopen(filepath, "w");
    if(f==NULL){
        perror("Opening file for server responses failed");
    }
    char system_call[200];
    snprintf(system_call, sizeof(system_call), "x-terminal-emulator -e \"tail -f %s\"", filepath);

    system(system_call);  // runs tail -f filepath on different console

    fprintf(f, "\n\nThis terminal is opened for outputting server responses to your requests\n\n");
    fflush(f);

    while(keepRunning) {
        recv(network_socket, &server_response, sizeof(server_response), 0);
        if(server_response[0]!='\0') {
            fprintf(f, "Server sent response: %s\n", server_response);
            fflush(f);
            strcpy(server_response, "\0");
        }
    }

    fclose(f);
    pthread_exit(0);
}
