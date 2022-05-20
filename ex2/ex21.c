#include "fcntl.h"
#include "stdlib.h"
#include "unistd.h"
#include "ctype.h"
#include "stdio.h"

int compareFiles(const char *fileA, const char *fileB, const int allowSimilar) {
    int fd_A = open(fileA, O_RDONLY);
    int fd_B = open(fileB, O_RDONLY);

    if (fd_A == -1 || fd_B == -1) {
        perror("Error in: open");
        exit(-1);
    }


    char a, b;
    size_t readA = read(fd_A, &a, 1), readB = read(fd_B, &b, 1);
    while (readA && readB) {
        if (allowSimilar) {
            if (a == ' ' || a == '\n') {
                readA = read(fd_A, &a, 1);
                continue;
            }
            if (b == ' ' || b == '\n') {
                readB = read(fd_B, &b, 1);
                continue;
            }

            a = tolower(a);
            b = tolower(b);
        }

        if (a != b) {
            close(fd_A);
            close(fd_B);
            return 0;
        }

        readA = read(fd_A, &a, 1);
        readB = read(fd_B, &b, 1);
    }

    if (readA || readB) {
        if (!allowSimilar) {
            close(fd_A);
            close(fd_B);
            return 0;
        }
        while (readA) {
            readA = read(fd_A, &a, 1);
            if (a != ' ' && a != '\n') {
                close(fd_A);
                close(fd_B);
                return 0;
            }
        }
        while (readB) {
            readB = read(fd_B, &b, 1);
            if (b != ' ' && b != '\n') {
                close(fd_A);
                close(fd_B);
                return 0;
            }
        }
    }

    close(fd_A);
    close(fd_B);
    return 1;
}

int main(int argc, char *argv[]) {
    char *file_A = argv[1];
    char *file_B = argv[2];

    if (compareFiles(file_A, file_B, 0)) {
        return 1;
    }
    if (compareFiles(file_A, file_B, 1)) {
        return 3;
    }
    return 2;
}
