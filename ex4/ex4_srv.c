#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "signal.h"

void clientHandler(int sig) {
    // Fork
    int pid_ = fork();
    if (pid_ == 0) {
        // Child process

        // Read from_srv file
        FILE *fp = fopen("to_srv", "r");

        // Read file
        int pid, first, operator, second;
        char *line = NULL;
        size_t len = 0;
        getline(&line, &len, fp);
        pid = atoi(line);
        getline(&line, &len, fp);
        first = atoi(line);
        getline(&line, &len, fp);
        operator = atoi(line);
        getline(&line, &len, fp);
        second = atoi(line);

        fclose(fp);
        unlink("to_srv");

        // Open file for writing
        char fileName[100];
        sprintf(fileName, "to_client_%d", pid);
        fp = fopen(fileName, "w");
        if (operator == 4 && second == 0) {
            fprintf(fp, "CANNOT_DIVIDE_BY_ZERO");
        } else {
            int result;
            switch (operator) {
                case 1:
                    result = first + second;
                    break;
                case 2:
                    result = first - second;
                    break;
                case 3:
                    result = first * second;
                    break;
                case 4:
                    result = first / second;
                    break;
            }
            fprintf(fp, "%d\n", result);
        }

        fclose(fp);
        kill(pid, SIGUSR1);
    }
}

void timeout(int sig) {
    printf("The server was closed because no service request was received for the last 60 seconds\n");
    while (wait(NULL) != -1);
    exit(1);
}

int main() {
    // Delete to_srv file if it exists
    unlink("to_srv");

    // Wait for client to signal server to start processing
    signal(SIGALRM, timeout);
    signal(SIGUSR1, clientHandler);
    while (1) {
        alarm(60);
        pause();
    }
}