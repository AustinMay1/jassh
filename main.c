#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define JASSH_RL_BUFSIZE 1024
#define JASSH_TOK_BUFSIZE 64
#define JASSH_TOK_DELIM " \t\r\n\a"

int jassh_launch(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();

    if(pid == 0)
    {
        if(execvp(args[0], args) == -1)
        {
            perror("jassh");
        }
        exit(EXIT_FAILURE);
    }
    else if(pid < 0)
    {
        perror("jassh");
    }
    else 
    {
        do 
        {
            wpid = waitpid(pid, &status, WUNTRACED);
        }
        while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int jassh_execute();

char **jassh_split_line(char *line) {
    int bufsize = JASSH_TOK_BUFSIZE;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char *)); // array of char *
                                                      // (strings)
    char *token;

    if(!tokens)
    {
        fprintf(stderr, "jassh: allocation error\n");
    }

    token = strtok(line, JASSH_TOK_DELIM); // tokenize every string delimited by
                                           // whitespace

    while(token != NULL) 
    {
        tokens[position] = token; // store the tokenized string
        position++;

        if(position >= bufsize)
        {
            bufsize += JASSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *)); // expand array
                                                                // if needed

            if(!tokens)
            {
                fprintf(stderr, "jassh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, JASSH_TOK_DELIM); // continue tokenizing until NULL
                                               // is returned. strtok knows to
                                               // continue parsing from the NULL
                                               // '\0' to the next delim.
    }
    tokens[position] = NULL; // null-terminate the array so execution knows when
                             // to stop reading in commands/flags
    return tokens;
};

char *jassh_read_line(void) {
   int bufsize = JASSH_RL_BUFSIZE; // initial buffer size 
   int position = 0;
   char *buffer = malloc(sizeof(char) * bufsize); // allocate buffer
   int c;

   if(!buffer)
   {
       fprintf(stderr, "jassh: allocation error\n");
       exit(EXIT_FAILURE);
   }

   while(1)
   {
       c = getchar(); // read in chars from stdin

       if (c == EOF || c == '\n')
       {
           buffer[position] = '\0'; // if eof or newline, terminate string and
                                    // return
           return buffer;
       }
       else 
       {
           buffer[position] = c; // build our string, add each char to buffer
       }
       position++;

       if(position >= bufsize) // if current position is greater than our
                               // buffersize 
       {
           bufsize += JASSH_RL_BUFSIZE;
           buffer = realloc(buffer, bufsize); // allocate some more memory and
                                              // continue building our string
           if(!buffer)
           {
               fprintf(stderr, "jassh: allocation error\n");
               exit(EXIT_FAILURE);
           }
       }
   }
};

void jassh(void)
{
    char *line;
    char **args;
    int status;

    do 
    {
        printf(">>> ");
        line = jassh_read_line();
        args = jassh_split_line(line);
        status = jassh_execute(args);

        free(line);
        free(args);
    }
    while (status);
}

int main(int argc, char **argv)
{
    jassh();

    return EXIT_SUCCESS;
}
