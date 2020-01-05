//
// Created by juuso on 7.11.2019.
//
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "miscellanous.h"
#include "queue.h"
#include "stdio.h"

#define NUM_OF_SERVERS 4

void min(struct Queue **sqs, int *min_value, int *min_index){
    int minimum;
    int index;
    minimum = sqs[0]->size;
    index = 0;
    for (int c = 1; sqs[c] != NULL; c++)
    {
        if (sqs[c]->size < minimum)
        {
            minimum = sqs[c]->size;
            index = c;
        }
    }
    *min_value = minimum;
    *min_index = index;
}

void wlog(const char *logtext){
    if(logtext[0] != '\0') {
        char filepath[] = "logfile.log";
        FILE *f = fopen(filepath, "a");

        if (-1 == dup2(fileno(f), fileno(stderr))) { perror("cannot redirect stderr"); }

        char buff[20];
        struct tm *sTm;

        time_t now = time(0);
        sTm = gmtime(&now);

        strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", sTm);

        char id[] = "<server>";
        fprintf(f, "%s %s %s\n", buff, id, logtext);
        fflush(f);
        fclose(f);
    }
}

