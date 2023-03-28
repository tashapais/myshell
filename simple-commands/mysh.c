#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

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

int execute_command(char **args, int arg_count) {
    if (!args[0]) {
        return 1;
    }

    if (strcmp(args[0], "exit") == 0) {
        exit(EXIT_SUCCESS);
    } else if (strcmp(args[0], "cd") == 0) {
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
    } else {
        pid_t pid = fork();
        if (pid == 0) {
            if (execvp(args[0], args) == -1) {
                perror(args[0]);
            }
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            perror("fork");
        } else {
            int status;
            waitpid(pid, &status, 0);
            return WEXITSTATUS(status);
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

    /*There is one input loop and command parsing algorithm that works for both batch 
    and interactive modes. The main difference between the two modes is the input source, 
    which is determined by the input_fd variable. When running in batch mode, 
    input_fd is set to the file descriptor of the specified input file. 
    When running in interactive mode, input_fd is set to the standard input file 
    descriptor DIN_FILENO) */
    
    char buffer[BUFFER_SIZE];
    char *args[MAX_ARGS];
    int arg_count;
    int exit_status;
    int input_fd = STDIN_FILENO;

    if (argc == 2) {
    //If there is an argument provided, the code opens the specified file 
    //and sets input_fd to the file descriptor of the opened file. 
    //Otherwise, it sets input_fd to the standard input file descriptor (STDIN_FILENO)
        input_fd = open(argv[1], O_RDONLY);
        if (input_fd == -1) {
            perror("mysh");
            exit(EXIT_FAILURE);
        }
    } else {
        write(STDOUT_FILENO, "Welcome to my shell!\n", 20);
    }

    while (1) {
    //The main input loop reads input one line at a time, regardless of the mode
    //In interactive mode, the shell prompt is printed before reading each line.
        if (input_fd == STDIN_FILENO) {
            print_prompt();
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
        last_exit_status = exit_status;
    }

    if (argc == 2) {
        close(input_fd);
    } else {
        write(STDOUT_FILENO, "mysh: exiting\n", 13);
    }

    return EXIT_SUCCESS;
}
