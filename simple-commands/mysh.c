#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <glob.h>

#define BUFFER_SIZE 1024
#define MAX_ARGS 64
#define DELIMITERS " \t\r\n\a|<>"

int last_exit_status = 0;

void expand_wildcards(char *pattern, char ***args, int *arg_count) {
    glob_t glob_result;

    glob(pattern, GLOB_NOCHECK, NULL, &glob_result);
    for (size_t i = 0; i < glob_result.gl_pathc; i++) {
        (*args)[*arg_count] = strdup(glob_result.gl_pathv[i]);
        (*arg_count)++;
    }

    globfree(&glob_result);
}

int parse_line(char *line, char **args) {
    int arg_count = 0;
    char *token = strtok(line, DELIMITERS);

    while (token != NULL) {
        if (strchr(token, '*') != NULL) {
            expand_wildcards(token, &args, &arg_count);
        } else {
            args[arg_count++] = token;
        }
        token = strtok(NULL, DELIMITERS);
    }
    args[arg_count] = NULL;
    return arg_count;
}

int handle_redirection(char **args, int *input_fd, int *output_fd) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0) {
            if (args[i + 1] != NULL) {
                *input_fd = open(args[i + 1], O_RDONLY);
                if (*input_fd < 0) {
                    perror(args[i + 1]);
                    return 1;
                }
                args[i] = NULL;
            } else {
                fprintf(stderr, "mysh: syntax error near unexpected token `<'\n");
                return 1;
            }
        } else if (strcmp(args[i], ">") == 0) {
            if (args[i + 1] != NULL) {
                *output_fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0640);
                if (*output_fd < 0) {
                    perror(args[i + 1]);
                    return 1;
                }
                args[i] = NULL;
            } else {
                fprintf(stderr, "mysh: syntax error near unexpected token `>'\n");
                return 1;
            }
        }
    }
    return 0;
}

int find_pipe_index(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            return i;
        }
    }
    return -1;
}

int execute_command(char **args, int arg_count) {
    int input_fd = STDIN_FILENO;
    int output_fd = STDOUT_FILENO;
    int pipe_fds[2];
    int pipe_index = find_pipe_index(args);

    if (arg_count == 0) {
        return 0;
    }
    if (pipe_index >= 0) {
        if (pipe(pipe_fds) == -1) {
            perror("mysh");
            return 1;
        }
        args[pipe_index] = NULL;
    }

    if (handle_redirection(args, &input_fd, &output_fd)) {
        return 1;
    }

    if (strcmp(args[0], "exit") == 0) {
        return -1;
    } else if (strcmp(args[0], "cd") == 0) {
        if (arg_count != 2) {
            write(STDERR_FILENO, "mysh: cd requires one argument\n", 31);
            return 1;
        }
        if (chdir(args[1]) != 0) {
            perror("mysh");
            return 1;
        }
        return 0;
    } else if (strcmp(args[0], "pwd") == 0) {
        if (arg_count != 1) {
            write(STDERR_FILENO, "mysh: pwd takes no arguments\n", 29);
            return 1;
        }
        char buf[BUFFER_SIZE];
        if (getcwd(buf, sizeof(buf)) == NULL) {
            perror("mysh");
            return 1;
        }
        write(STDOUT_FILENO, buf, strlen(buf));
        write(STDOUT_FILENO, "\n", 1);
        return 0;
    }

    int child_pid = fork();

    if (child_pid == 0) {
        // Child process
        if (input_fd != STDIN_FILENO) {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }
        if (output_fd != STDOUT_FILENO) {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }

        // Handling pipes
        if (pipe_index >= 0) {
            int child2_pid = fork();

            if (child2_pid == 0) {
                // Child process for the second command
                close(pipe_fds[1]); // Close the write end
                dup2(pipe_fds[0], STDIN_FILENO); // Redirect stdin to the read end of the pipe
                close(pipe_fds[0]);
                execvp(args[pipe_index + 1], args + pipe_index + 1);
                perror(args[pipe_index + 1]);
                exit(1);
            }

            // In the first child process
            close(pipe_fds[0]); // Close the read end
            dup2(pipe_fds[1], STDOUT_FILENO); // Redirect stdout to the write end of the pipe
            close(pipe_fds[1]);
        }

        execvp(args[0], args);
        perror(args[0]);
        exit(1);
    } else if (child_pid == -1) {
        perror("mysh");
        return 1;
    } else {
        // Parent process
        int status;
        waitpid(child_pid, &status, 0);

        if (input_fd != STDIN_FILENO) {
            close(input_fd);
        }
        if (output_fd != STDOUT_FILENO) {
            close(output_fd);
        }

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return 1;
        }
    }
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

    write(STDOUT_FILENO, "mysh: exiting\n", 14);

    if (!is_interactive) {
        close(input_fd);
    }

    return EXIT_SUCCESS;
}
