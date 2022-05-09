// Tom Ben-Dor
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

void split(char *line, char** dest) {
    char *token = strtok(line, " ");
    int i = 0;
    while (token != NULL) {
        dest[i++] = token;
        token = strtok(NULL, " ");
    }
    dest[i] = NULL;
}

int main(int argc, char *argv[]) {
    // Adding all arguments to path environment variable
    char *path = getenv("PATH");
    int i;
    for (i = 1; i < argc; i++) {
        path = strcat(path, ":");
        path = strcat(path, argv[i]);
    }
    setenv("PATH", path, 1);

    char *commands_history[101];
    int pid_history[101];

    int history_index;
    for (history_index = 0;; history_index++){
        printf("$ ");
        fflush(stdout);
        char command[101] = "";
        scanf(" %[^\n]", command);
        char* args[100] = {};
        split(command, args);

        commands_history[history_index] = strdup(command);
        pid_history[history_index] = getpid();

        if (strcmp(args[0], "exit") == 0) {
            break;
        }

        if (strcmp(args[0], "cd") == 0) {
            if (chdir(args[1]) == -1) {
                perror("chdir failed");
                return 1;
            }
            continue;
        }

        if (strcmp(args[0], "history") == 0) {
            int i;
            for (i = 0; i <= history_index; i++) {
                printf("%d %s\n", pid_history[i], commands_history[i]);
            }
            continue;
        }

        int pid = fork();
        if (pid < 0) {
            perror("fork failed");
            return 1;
        }

        if (pid == 0) {
            int result = execvp(args[0], args);
            if (result == -1) {
                perror("execvp failed");
                return 1;
            }
        } else {
            pid_history[history_index] = pid;
            wait(NULL);
        }
    }
}