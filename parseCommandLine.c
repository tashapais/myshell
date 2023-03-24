#include "myShell.h"

void* parseCommandLine(char *tokens[]){
    int bytes_read, i=0, token=0;
    size_t nbytes = MAX_LENGTH + 2; /*accounts for end of string '\0' char*/
    char *my_string;

    /*malloc enough space for a MAX_LENGTH character line*/
    my_string = (char *)malloc(nbytes + 1);
    if(my_string == NULL){ /*malloc fails*/
        return NULL;
    }

    /*Get the input line from user*/
    /*getline() also reads the newline '\n' character*/
    bytes_read = getline (&my_string, &nbytes, stdin);
    if(bytes_read == -1){ /*If getline() failed*/
        free(my_string); /* free up the space -- no memory leaks!*/
        fprintf(stderr, "getline() failed\n");
        return NULL;
    }
    if(bytes_read > MAX_LENGTH+1){ /*If the input is too long*/
        free(my_string); /* free up the space -- no memory leaks!*/
        fprintf(stderr, "Too many characters in command\n");
        return NULL;
    }

    do{
        /*Truncate space and tab characters*/
        while(my_string[i]==' ' || my_string[i]=='\t'){
            my_string[i]='\0';
            i++;
        }

        /*If '\n' is reached (end of input)*/
        if(my_string[i]=='\n'){
            if(token > MAX_TOKENS){ /*Too many number of tokens*/
                free(my_string); /* free up the space*/
                fprintf(stderr, "Too many arguments\n");
                return NULL;
            }
            /*replace '\n' with '\0' (end of string character*/
            my_string[i]='\0';
            /*end token string with NULL so that execvp will work*/
            tokens[token]=NULL;
            /* return the pointer to the malloced memory so that call can free it*/
            return my_string;
        }
    /*char not a newline. It must be the start of new token.*/
    tokens[token]=&my_string[i];
    token++;
    i++;
    
    /*get the rest of the characters in this token*/
    /*isgraph() determines if a character is printable and a non-space*/
    while(isgraph(my_string[i]) && my_string[i] != '\n'){
        i++;
    }
    }while(1);
}
