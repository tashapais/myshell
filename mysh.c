#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>













int main(int argc, char** argv){
    FILE *batchfile = NULL;
    int errorFlag = 0

    if (argc == 2){
        batchfile = fopen(argv[1], "r");
        if (batchfile == NULL){
            perror("Failed to open batchfile");
            exit(1);
        }
    }

    char line = malloc(sizeof(char)*256);
    while(1){
        if (batchfile == NULL){
            if (errorFlag == 0){
                printf("mysh>");
            }else{
                printf("!mysh>");
            }
            if (read(line, sizeof(line, stdin) == NULL)){
                break;
            }
        }else{
            if(fgets(line, sizeof(line, stdin) == NULL)){
                break;
            }
            printf("%s", line)
        }
    }
    
}