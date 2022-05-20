#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <dirent.h>
#include <stdio.h>

void readline(int fd, char *buf) {
    int i = 0;
    char c;

    while (read(fd, &c, 1) > 0) {
        if (c == '\n')
            break;

        buf[i++] = c;
    }

    buf[i] = '\0';
}

int isFolder(const char *path) {
    struct stat st;

    if (stat(path, &st) == -1)
        return 0;

    return S_ISDIR(st.st_mode);
}

int isFile(const char *path) {
    struct stat st;

    if (stat(path, &st) == -1)
        return 0;

    return S_ISREG(st.st_mode);
}

int main(int argc, char *argv[]) {
    char *config_file = argv[1];

    // Read the config file
    int fd = open(config_file, O_RDONLY);

    char students_folder[151] = "", input_file[151] = "", correct_output_file[151] = "";
    readline(fd, students_folder);
    readline(fd, input_file);
    readline(fd, correct_output_file);

    close(fd);

    // Verify students_folder exists and is a directory
    if (!isFolder(students_folder)) {
        const char *err_msg = "Not a valid directory\n";
        write(1, err_msg, strlen(err_msg));
        return -1;
    }

    // Verify input_file exists and is a file
    if (!isFile(input_file)) {
        const char *err_msg = "Input file not exist\n";
        write(1, err_msg, strlen(err_msg));
        return -1;
    }

    // Verify correct_output_file exists and is a file
    if (!isFile(correct_output_file)) {
        const char *err_msg = "Output file not exist\n";
        write(1, err_msg, strlen(err_msg));
        return -1;
    }

    // Create error.txt file
    int errorsFD = open("errors.txt", O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);
    if (errorsFD == -1) {
        perror("Error in: open");
    }

    // Create results.csv file
    int resultsFD = open("results.csv", O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);
    if (resultsFD == -1) {
        perror("Error in: open");
    }

    int inputFD = open(input_file, O_RDWR);
    if (inputFD == -1) {
        perror("Error in: open");
    }

    // Open the students_folder
    DIR *studentsDir = opendir(students_folder);

    // Iterate over the students_folder
    struct dirent *entry;
    while ((entry = readdir(studentsDir)) != NULL) {
        // Skip the . and .. folders
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char student_folder[151] = "";
        strcat(student_folder, students_folder);
        strcat(student_folder, "/");
        strcat(student_folder, entry->d_name);

        if (!isFolder(student_folder))
            continue;

        // Open the student_folder
        DIR *studentDir = opendir(student_folder);

        // Iterate over the student_folder
        int foundC = 0;
        int outputFD = open("output.txt", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
        if (outputFD == -1) {
            perror("Error in: open");
            continue; // Maybe it'll work for the next student
        }

        struct dirent *entry2;
        while ((entry2 = readdir(studentDir)) != NULL) {
            // Skip the . and .. folders
            if (strcmp(entry2->d_name, ".") == 0 || strcmp(entry2->d_name, "..") == 0)
                continue;

            char student_file[151] = "";
            strcat(student_file, student_folder);
            strcat(student_file, "/");
            strcat(student_file, entry2->d_name);

            if (!isFile(student_file))
                continue;

            // Check if student_file is a c file
            if (strcmp(entry2->d_name + strlen(entry2->d_name) - 2, ".c") != 0)
                continue;

            foundC = 1;
            // Compile the student_file on a child process
            pid_t pid = fork();
            if (pid < 0) {
                perror("Error in: fork");
                continue; // Maybe it'll work for the next student
            }
            if (pid == 0) {
                dup2(errorsFD, STDERR_FILENO);
                execlp("gcc", "gcc", student_file, NULL);
            } else {
                int status;
                wait(&status);
                // Check if the compilation was successful
                if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                    write(resultsFD, entry->d_name, strlen(entry->d_name));
                    write(resultsFD, ",10,COMPILATION_ERROR\n", strlen(",10,COMPILATION_ERROR\n"));
                    break;
                }
            }

            pid = fork();
            if (pid < 0) {
                perror("Error in: fork");
                continue; // Maybe it'll work for the next student
            }
            if (pid == 0) {
                lseek(inputFD, 0, SEEK_SET);
                dup2(inputFD, STDIN_FILENO);
                dup2(outputFD, STDOUT_FILENO);
                dup2(errorsFD, STDERR_FILENO);

                execl("a.out", "a.out", NULL);
            } else {
                int status;
                wait(&status);
            }

            // Compare the output with the correct output
            pid = fork();
            if (pid < 0) {
                perror("Error in: fork");
                continue; // Maybe it'll work for the next student
            }
            if (pid == 0) {
                execl("comp.out", "comp.out", "output.txt", correct_output_file, NULL);
                perror("Error in: exec");
            } else {
                int status;
                wait(&status);

                int exit_status = WEXITSTATUS(status);

                switch (exit_status) {
                    case 1:
                        write(resultsFD, entry->d_name, strlen(entry->d_name));
                        write(resultsFD, ",100,EXCELLENT\n", strlen(",100,EXCELLENT\n"));
                        break;
                    case 2:
                        write(resultsFD, entry->d_name, strlen(entry->d_name));
                        write(resultsFD, ",50,WRONG\n", strlen(",50,WRONG\n"));
                        break;
                    case 3:
                        write(resultsFD, entry->d_name, strlen(entry->d_name));
                        write(resultsFD, ",75,SIMILAR\n", strlen(",75,SIMILAR\n"));
                        break;
                }
            }


            break; // We only need to check the first c file
        }
        closedir(studentDir);
        if (close(outputFD)) {
            perror("Error in: close");
        }

        if (remove("output.txt")) {
            perror("Error in: remove");
            return -1; // This will be a problem
        }
        remove("a.out"); // Error is OK

        if (!foundC) {
            write(resultsFD, entry->d_name, strlen(entry->d_name));
            write(resultsFD, ",0,NO_C_FILE\n", strlen(",0,NO_C_FILE\n"));
        }
    }

    closedir(studentsDir);

    if (close(inputFD)) {
        perror("Error in: close");
    }
    if (close(errorsFD)) {
        perror("Error in: close");
    }
}