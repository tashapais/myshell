#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>

#define BUFFER_SIZE 1024
#define MAX_ARGS 64
#define DELIMITERS " \t\r\n\a"

int last_exit_status = 0;

int parse_line(char *line, char **args) {
    int arg_count = 0;
    char *token = strtok(line, DELIMITERS);

    while (token != NULL) {
        args[arg_count++] = token;
        token = strtok(NULL, DELIMITERS);
    }
    args[arg_count] = NULL;
    return arg_count;
}

int execute_command(char *args[], int arg_count) {
    if (arg_count == 0) {
        return 0;
    }

    if (strcmp(args[0], "cd") == 0) {
        if (arg_count > 1) {
            if (chdir(args[1]) == -1) {
                perror("cd");
                return 1;
            }
        } else {
            fprintf(stderr, "cd: missing operand\n");
            return 1;
        }
        return 0;
    }

    if (strcmp(args[0], "echo") == 0) {
        for (int i = 1; i < arg_count; i++) {
            write(STDOUT_FILENO, args[i], strlen(args[i]));
            write(STDOUT_FILENO, " ", 1);
        }
        write(STDOUT_FILENO, "\n", 1);
        return 0;
    }

    if (strcmp(args[0], "exit") == 0) {
        return -1;
    }

    if (strcmp(args[0], "mkdir") == 0) {
        if (arg_count > 1) {
            if (mkdir(args[1], 0755) == -1) {
                perror("mkdir");
                return 1;
            }
        } else {
            fprintf(stderr, "mkdir: missing operand\n");
            return 1;
        }
        return 0;
    }

    if (strcmp(args[0], "pwd") == 0) {
        char cwd[PATH_MAX];
        if (getcwd(cwd, PATH_MAX) != NULL) {
            write(STDOUT_FILENO, cwd, strlen(cwd));
            write(STDOUT_FILENO, "\n", 1);
        } else {
            perror("pwd");
            return 1;
        }
        return 0;
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("mysh");
        return 1;
    }

    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror(args[0]);
            exit(EXIT_FAILURE);
        }
    } else {
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("mysh");
            return 1;
        }
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            fprintf(stderr, "%s terminated by signal %d\n", args[0], WTERMSIG(status));
            return 1;
        }
    }

    return 0;
}


void print_prompt() {
    if (last_exit_status == 0) {
        write(STDOUT_FILENO, "mysh> ", 6);
    } else {
        write(STDOUT_FILENO, "!mysh> ", 7);
    }
}

int main(int argc, char *argv[]) {
    char buffer[BUFFER_SIZE];
    char *args[MAX_ARGS];
    int arg_count;
    int exit_status;
    int input_fd = STDIN_FILENO;
    int is_interactive = (argc == 1);

    if (!is_interactive) {
        input_fd = open(argv[1], O_RDONLY);
        if (input_fd == -1) {
            perror("mysh");
            exit(EXIT_FAILURE);
        }
    } else {
        write(STDOUT_FILENO, "Welcome to my shell!\n", 22);
    }

    while (1) {
        if (is_interactive) {
            if (last_exit_status != 0) {
                write(STDOUT_FILENO, "!mysh> ", 7);
            } else {
                write(STDOUT_FILENO, "mysh> ", 6);
            }
        }

        ssize_t read_size = 0;
        size_t buffer_idx = 0;

        while (1) {
            char c;
            read_size = read(input_fd, &c, 1);

            if (read_size == -1) {
                perror("mysh");
                break;
            }

            if (read_size == 0 || c == '\n') {
                buffer[buffer_idx] = '\0';
                break;
            }

            buffer[buffer_idx++] = c;
        }

        if (read_size == 0) {
            break;
        }

        arg_count = parse_line(buffer, args);
        exit_status = execute_command(args, arg_count);

        if (exit_status == -1) {
            break;
        }

        last_exit_status = exit_status;
    }

    if (!is_interactive) {
        close(input_fd);
    }

    write(STDOUT_FILENO, "mysh: exiting\n", 13);

    return EXIT_SUCCESS;
}
