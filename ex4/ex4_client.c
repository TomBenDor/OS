#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

void timeout(int sig) {
    printf("Client closed because no response was received from the server for 30 seconds\n");
    exit(1);
}

void processResponse(int sig) {
    char fileName[100];
    sprintf(fileName, "to_client_%d", getpid());

    // Open file for reading
    FILE *fp = fopen(fileName, "r");

    // Read file
    char *line = NULL;
    size_t len = 0;
    getline(&line, &len, fp);

    fclose(fp);
    unlink(fileName);

    printf("%s\n", line);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("ERROR_FROM_EX4\n");
        exit(1);
    }

    int server_pid = atoi(argv[1]);
    char *P2 = argv[2], *P3 = argv[3], *P4 = argv[4];

    int tries = 0;
    // Check if to_srv file exists
    while (access("to_srv", F_OK) != -1) {
        if (++tries > 10) {
            printf("Server is not ready\n");
            exit(1);
        }
        sleep(rand() % 6);
    }
    FILE *fp = fopen("to_srv", "w");

    // Write to_srv file
    fprintf(fp, "%d\n%s\n%s\n%s", getpid(), P2, P3, P4);

    // Close to_srv file
    fclose(fp);

    // Signal server to start processing
    kill(server_pid, SIGUSR1);

    // Wait for server to finish processing
    signal(SIGALRM, timeout);
    signal(SIGUSR1, processResponse);
    alarm(60);
    pause();
}