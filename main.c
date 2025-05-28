#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

extern char **environ;

#define MAX_LINE 1024
#define MAX_ARGS 64

int split_pipe(char *args[], char *left[], char *right[]) {
    int i = 0;
    while (args[i] && strcmp(args[i], "|") != 0) {
        left[i] = args[i];
        i++;
    }
    if (!args[i]) return 0;
    left[i] = NULL;
    i++;
    int j = 0;
    while (args[i]) {
        right[j++] = args[i++];
    }
    right[j] = NULL;
    return 1;
}

int main() {
    char line[MAX_LINE];
    char *args[MAX_ARGS];
    int status = 1;

    while (status) {
        printf("$> ");
        if (fgets(line, MAX_LINE, stdin) == NULL)
            break;
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0) continue;

        int i = 0;
        args[i] = strtok(line, " ");
        while (args[i] != NULL && i < MAX_ARGS - 1) {
            i++;
            args[i] = strtok(NULL, " ");
        }
        args[i] = NULL;

        int background = 0;
        if (i > 0 && args[i - 1] && strcmp(args[i - 1], "&") == 0) {
            background = 1;
            args[i - 1] = NULL;
        }

        // exit
        if (strcmp(args[0], "exit") == 0) {
            printf("Bye!\n");
            status = 0;
            continue;
        }

        // cd
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL)
                fprintf(stderr, "cd : chemin manquant\n");
            else if (chdir(args[1]) != 0)
                perror("cd");
            continue;
        }

        // env
        if (strcmp(args[0], "env") == 0) {
            for (int j = 0; environ[j]; j++) {
                printf("%s\n", environ[j]);
            }
            continue;
        }

        // pipe
        char *left[MAX_ARGS], *right[MAX_ARGS];
        if (split_pipe(args, left, right)) {
            int fd[2];
            if (pipe(fd) == -1) {
                perror("pipe");
                continue;
            }

            pid_t pid1 = fork();
            if (pid1 == 0) {
                dup2(fd[1], STDOUT_FILENO);
                close(fd[0]);
                close(fd[1]);
                execvp(left[0], left);
                perror("exec");
                exit(1);
            }

            pid_t pid2 = fork();
            if (pid2 == 0) {
                dup2(fd[0], STDIN_FILENO);
                close(fd[1]);
                close(fd[0]);
                execvp(right[0], right);
                perror("exec");
                exit(1);
            }

            close(fd[0]);
            close(fd[1]);
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
            continue;
        }

        // fork + redirection + exec
        pid_t pid = fork();
        if (pid == 0) {
            for (int j = 0; args[j]; j++) {
                if (strcmp(args[j], ">") == 0 && args[j + 1]) {
                    int fd = open(args[j + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd < 0) {
                        perror("open");
                        exit(1);
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    args[j] = NULL;
                    break;
                }
                if (strcmp(args[j], "<") == 0 && args[j + 1]) {
                    int fd = open(args[j + 1], O_RDONLY);
                    if (fd < 0) {
                        perror("open");
                        exit(1);
                    }
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                    args[j] = NULL;
                    break;
                }
            }

            execvp(args[0], args);
            perror("exec");
            exit(1);
        } else if (pid > 0) {
            if (!background)
                wait(NULL);
        } else {
            perror("fork");
        }
    }

    return 0;
}
