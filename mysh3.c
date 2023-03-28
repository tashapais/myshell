#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <fcntl.h>
  
#define MAXCOM 1000 // max number of letters to be supported
#define MAXLIST 100 // max number of commands to be supported

int errorFlag = 0;

// struct StringList{
//     char *data;
//     struct StringList *nextLine;
// };

// struct StringList *head;
  
// Greeting shell during startup
void init_shell()
{
    //clear();
   puts("\n\n\n\n******************"
        "************************");
    puts("\n\n\n****MY SHELL INTERACTIVE MODE****");
    puts("\n\n\t-USE AT YOUR OWN RISK-");
    puts("\n\n\n\n*******************"
        "***********************");
    puts("\n");
    sleep(1);
    //clear();
}
  
// Function to take input
int takeInput(char* str)
{
    char* buf;
    if (errorFlag != 0){
        buf = readline("\n!mysh> ");
    }else{
        buf = readline("\nmysh> ");
    }
    if (strlen(buf) != 0) {
        add_history(buf);
        strcpy(str, buf);
        return 0;
    } else {
        return 1;
    }
}
  
// Function to print Current Directory.
void printDir()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    //puts("\nDir: ");
    fprintf(stdout,"Dir: %s", cwd);
}
  
// Function where the system command is executed
void execArgs(char** parsed)
{
    // Forking a child
    pid_t pid = fork(); 
  
    if (pid == -1) {
        perror("\nFailed forking child..");
        errorFlag = 1;
        return;
    } else if (pid == 0) {
        //errorFlag = 0;
        if (execvp(parsed[0], parsed) < 0) {
            perror("\nCould not execute command..");
            errorFlag = 1;
        }
        exit(0);
    } else {
        // waiting for child to terminate
        wait(NULL); 
        return;
    }
}
  
// Function where the piped system commands is executed
void execArgsPiped(char** parsed, char** parsedpipe)
{
    // 0 is read end, 1 is write end
    int pipefd[2]; 
    pid_t p1, p2;
    

    if (pipe(pipefd) < 0) {
        perror("\nPipe could not be initialized");
        errorFlag = 1;
        return;
    }
    p1 = fork();
    if (p1 < 0) {
        perror("\nCould not fork");
        errorFlag = 1;
        return;
    }
    printf("before if");
    printf("p1: %d", p1==0);
    if (p1 == 0) {
        puts("past if");
        // Child 1 executing..
        // It only needs to write at the write end
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
  
        //errorFlag = 0;
        fprintf(stdout, "parsed 3: %s", *parsed);
        fprintf(stdout,"parsed[0] : %s", parsed[0]);
        if (execvp(parsed[0], parsed) < 0) {
            perror("\nCould not execute command 1");
            errorFlag = 1;
            exit(0);
        }
    } else {
        printf("in else");
        // Parent executing
        p2 = fork();
  
        if (p2 < 0) {
            perror("\nCould not fork");
            errorFlag = 1;
            return;
        }
  
        // Child 2 executing..
        // It only needs to read at the read end
        if (p2 == 0) {
            //errorFlag = 0;
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            if (execvp(parsedpipe[0], parsedpipe) < 0) {
                perror("\nCould not execute command 2..");
                errorFlag = 1;
                exit(0);
            }
        } else {
            // parent executing, waiting for two children
            wait(NULL);
            wait(NULL);
        }
    }
}
  
// Help command builtin
void openHelp()
{
    puts("\n***WELCOME TO MY SHELL HELP***"
        "\n-Use the shell at your own risk..."
        "\nList of Commands supported:"
        "\n>cd"
        "\n>ls"
        "\n>exit"
        "\n>all other general commands available in UNIX shell"
        "\n>pipe handling"
        "\n>redirection handling"
        "\n>improper space handling");
  
    return;
}
  
// Function to execute builtin commands
int ownCmdHandler(char** parsed)
{
    int NoOfOwnCmds = 3, i, switchOwnArg = 0;
    char* ListOfOwnCmds[NoOfOwnCmds];
    char* username;
  
    ListOfOwnCmds[0] = "exit";
    ListOfOwnCmds[1] = "cd";
    ListOfOwnCmds[2] = "pwd";
  
    for (i = 0; i < NoOfOwnCmds; i++) {
        if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) {
            switchOwnArg = i + 1;
            break;
        }
    }
  
    switch (switchOwnArg) {
    case 1:
        puts("\nGoodbye\n");
        exit(0);
    case 2:
        chdir(parsed[1]);
        //errorFlag = 0;
        return 1;
    case 3:
        printDir();
        //errorFlag = 0;
        return 1;
    default:
        errorFlag = 1;
        break;
    }
  
    return 0;
}
  
// function for finding pipe
int parsePipe(char* str, char** strpiped)
{
    int i;
    for (i = 0; i < 2; i++) {
        strpiped[i] = strsep(&str, "|");
        if (strpiped[i] == NULL)
            break;
    }
  
    if (strpiped[1] == NULL)
        return 0; // returns zero if no pipe is found.
    else {
        return 1;
    }
}
  
// function for parsing command words
void parseSpace(char* str, char** parsed)
{
    //printf("parsed 2: %s", *parsed);
    int i;
  
    for (i = 0; i < MAXLIST; i++) {
        parsed[i] = strsep(&str, " ");
  
        if (parsed[i] == NULL)
            break;
        if (strlen(parsed[i]) == 0)
            i--;
    }
    //printf("parsed 2: %s", *parsed);
}
  
int processString(char* str, char** parsed, char** parsedpipe)
{
    //printf("Parsed: %s", *parsed);
    char* strpiped[2];
    int piped = 0;
  
    piped = parsePipe(str, strpiped);
  
    if (piped) {
        parseSpace(strpiped[0], parsed);
        parseSpace(strpiped[1], parsedpipe);
  
    } else {
  
        parseSpace(str, parsed);
    }
  
    if (ownCmdHandler(parsed))
        return 0;
    else
        return 1 + piped;
}
  
int main(int argc, char** argv)
{
    char inputString[MAXCOM], *parsedArgs[MAXLIST];
    char* parsedArgsPiped[MAXLIST];
    int execFlag = 0;
    int batchFile = -1;

    if (argc == 2){
        batchFile = open(argv[1], O_RDONLY);
        if (batchFile == -1){
            perror("Failed to open batchfile");
            errorFlag = 1;
            exit(1);
        }
    }
  
    while (1) {
        //handle batch mode
        //printDir();
        if (batchFile == -1){
            // take input
            init_shell();
            if (takeInput(inputString))
            continue;
        }else{
            // take input
            char buffer[2048];
            read(batchFile, buffer, sizeof(buffer));
            //printf("buffer: %s", buffer);
            
            continue;
        }
        // take input
        // if (takeInput(inputString))
        //     continue;
        // process


        execFlag = processString(inputString, parsedArgs, parsedArgsPiped);
        // execflag returns zero if there is no command
        // or it is a builtin command,
        // 1 if it is a simple command
        // 2 if it is including a pipe.
  
        // execute
        if (execFlag == 1)
            execArgs(parsedArgs);
            errorFlag = 0;
  
        if (execFlag == 2)
            execArgsPiped(parsedArgs, parsedArgsPiped);
            errorFlag = 0;
    }
    return 0;
}