#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>

int execute_command(char **args, int arg_count) {
    //printf("Executing command: %s\n", args[0]);

    if (!args[0]) {
        return 1;
    }

    if (strcmp(args[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(args[0], "cd") == 0) { //checks if the first argument (the command name) in the args array is equal to the string "cd"
        if (arg_count > 1) { //ensure that there is at least one additional argument provided (i.e., the target directory)
            if (chdir(args[1]) == -1) { //chdir() function is called with the second argument in the args array, which should be the target directory
                perror("cd"); //If the chdir() call is successful, it returns 0. If it fails, it returns -1.
                return 1; //function returns 1 to indicate that the command failed
            }
        } else {
            fprintf(stderr, "cd: missing operand\n");
            return 1;
            //If the arg_count is not greater than 1, meaning no additional argument was provided for the cd command, 
            //an error message is printed to stderr stating that the operand is missing, 
            //and the function returns 1 to indicate the command failed.
        }
        return 0;
        //If the chdir() function call was successful, the function returns 0 
        //to indicate that the cd command was executed without any issues
    } else if (strcmp(args[0], "pwd") == 0) {
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        printf("%s\n", cwd);
        return 0;
    } else if (strcmp(args[0], "echo") == 0) {
        for (int i = 1; i < arg_count; i++) {
            printf("%s ", args[i]);
        }
        printf("\n");
        return 0;
    } else if (strcmp(args[0], "mkdir") == 0) {
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
    } else if (strcmp(args[0], "rmdir") == 0) {
        if (arg_count > 1) {
            if (rmdir(args[1]) == -1) {
                perror("rmdir");
                return 1;
            }
        } else {
            fprintf(stderr, "rmdir: missing operand\n");
            return 1;
        }
        return 0;
    } else {
        pid_t pid = fork();
        if (pid == 0) {
            execvp(args[0], args);
            perror("execvp");
            exit(1);
        } else {
            int status;
            waitpid(pid, &status, 0);
            return WEXITSTATUS(status);
        }
    }
}


int main(int argc, char *argv[]) {
    FILE *input_file = stdin;

    if (argc == 2) {
        input_file = fopen(argv[1], "r");
        if (!input_file) {
            perror("fopen");
            exit(1);
        }
    } else if (argc > 2) {
        fprintf(stderr, "Usage: %s [script_file]\n", argv[0]);
        exit(1);
    }

    if (input_file == stdin) {
        printf("Welcome to my shell!\n");
    }

    char buffer[4096];
    int last_command_failed = 0;

    while (1) {
        //printf("Reading input...\n");
        if (input_file == stdin) {
            printf("%smysh> ", last_command_failed ? "!" : "");
        }

        if (fgets(buffer, sizeof(buffer), input_file) == NULL || feof(input_file)) {
            break;
        }

        char *args[64];
        int arg_count = 0;

        char *token = strtok(buffer, " \n");
        while (token) {
            args[arg_count++] = token;
            token = strtok(NULL, " \n");
        }
        args[arg_count] = NULL;

        last_command_failed = execute_command(args, arg_count);
    }

    if (input_file == stdin) {
        printf("mysh: exiting\n");
    }

    return 0;
}
