#include <stdio.h>
#include <stdlib.h>

int main(){
    FILE *batchfile = NULL;

    //Check if a batchfile was specified
    if (argc == 2){
        //Open batchfile for reading
        batchfile = fopen(argv[1], "r");
        if  (batchfile == NULL){
            perror("Failed to open batchfile");
            exit(1);
        }
    }

    //Process commands
    char line[256];
    while (1) {
        if (batchfile == NULL){
            printf("mysh> ");
            if (fgets(line, sizeof(line, stdin) == NULL)){
                break;
            }
        }else{
            if ((fgets(line, sizeof(line, stdin) == NULL))){
                break;
            }
            printf("%s", line);
        }


        // Remove newLine character from input
        line[strcspn(line, "\n")] = '\0';
        if (strcmp(line, "exit") == 0){
            break;
        }

        //Process the command


        //Split the command line into arguments
        char *args[32];
        int argc = 0;
        char *token = strtok(line, " ");
        while (token != NULL && argc < 31){
            args[argc++] = token;
            token = strtok(NULL, " ");
        }
        args[argc] = NULL;

        // Create a child process to execute the command
        pid_t pid = fork();
        if (pid == -1){
            perror("Failed to fork");
            exit(1);
        }else if (pid == 0){
            //This is the child process

            // Execute the command
            execvp(args[0], args);

            // If execvp returns, there was an error
            perror("Failed to execute command");
            exit(1);
        }else{
            // This is the parent process

            //Wait for the child process to finish
            int status;
            waitpid(pid, &status, 0);
        }
    }


    // Close the batch file
    if (batchfile != NULL){
        fclose(batchfile);
    }

    return 0
}



int main(){
    int fd[2];
    int ret = pipe(fd);
    if(ret == -1){
        perror("pipe error");
        exit(1);
    }
    int i;
    pid_t pid, wpid;
    for (i = 0; i < 2; i++){
        if ((pid = fork()) == 0){
            break;
        }
    }
    if (i == 2){
        close(fd[0]);
        clode(fd[1]);
        wpid = wait(NULL);
        printf("wait child 1 success, pid = %d\n", wpid);
        pid = wait(NULL);
        printf("wait child 2 success, pid = %d\n", pid);
    }else if(i == 0){
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        execlp("ls", "ls", NULL);
    }else if(i == 1){
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        execlp("wc", "wc","-l", NULL);
    }
    return 0;

}    

        char **args;
        /*
        do something to format the commands
        */

        if(/*we can find a bar in the command*/){
            //Create pipe
            if (pipe(fd) == -1){
                perror("pipe");
                exit(EXIT_FAILURE);
            }

            //Fork child process
            pid = fork();
            if (pid == -1){
                perror("fork");
                exit(EXIT_FAILURE);
            }

            //Child process executes first command and write output to pipe
            if (pid == 0){
                close(fd[0]); // close unused reaed end of pipe

                //Redirect stdout to write end of pipe
                if(dup2(fd[1], STDOUT_FILENO) == -1){
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
                
                //Execute first command
                /*
                do somehting here
                */
                perror("execvp");
                exit(EXIT_FAILURE);
            }

            //Parent process executes second command and reads inout from pipe
            else{
                //Wait for child process to finish
                wait(NULL);

                //close unused write pipe
                close(fd[1]);

                //redirect stdin to read end of pipe
                if(dup2(fd[0], STDIN_FILENO) == -1){
                    perrer("dup2");
                    exit(EXIT_FAILURE);
                }


                //Parse second command
                /*
                do something here
                */

                //Execute second command
                /*
                do somehing here
                */

                perror("execvp");
                exit(EXIT_FAILURE);

            }
        }