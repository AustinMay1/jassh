#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define JASSH_RL_BUFSIZE 1024
#define JASSH_TOK_BUFSIZE 64
#define JASSH_TOK_DELIM " \t\r\n\a"

int jassh_cd(char **args);
int jassh_help(char **args);
int jassh_exit(char **args);

char *builtin_cmd[] = { "cd", "help", "exit" };

int (*builtin_func[]) (char **) = { &jassh_cd, &jassh_help, &jassh_exit }; // function pointers

int jassh_ls_builtins() 
{
    return sizeof(builtin_cmd) / sizeof(char *);
}

int jassh_cd(char **args) 
{
    if(args[1] == NULL) 
    {
        fprintf(stderr, "jassh: expected argument to \"cd\"\n");
    } 
    else 
    {
        if (chdir(args[1]) != 0) 
        {
            perror("jassh");
        }
    }

    return 1;
}

int jassh_help(char **args) 
{
    int i;
    printf("=====JASSH=====\n");
    printf("The following functions are builtin:\n");

    for(i = 0; i < jassh_ls_builtins(); i++) 
    {
        printf(" %s\n", builtin_cmd[i]);
    }

    return 1;
}

int jassh_exit(char **args) 
{
    return 0;
}

int jassh_launch(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork(); // create copy of the process
    printf("[pid]: %d\n", pid); // just for me to see 

    if(pid == 0)
    {
        if(execvp(args[0], args) == -1) // v:program name + array of args, p:let os find the program, execvp should not return anything ideally
        {
            perror("jassh");
        }
        exit(EXIT_FAILURE);
    }
    else if(pid < 0) // fork should not error but if so
    {
        perror("jassh");
    }
    else 
    {
        do 
        {
            wpid = waitpid(pid, &status, WUNTRACED); // wait for our spawned process to finish. either by execution or signal
        }
        while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int jassh_execute(char **args)
{
    int i;

    if(args[0] == NULL)
    {
        return 1;
    }

    for(i = 0; i < jassh_ls_builtins(); i++)
    {
        if(strcmp(args[0], builtin_cmd[i]) == 0) // if command provided is in built-in cmd array
        {
            return (*builtin_func[i]) (args); // call its function
        }
    }

    return jassh_launch(args); // if not a builtin, find and spawn the process.
};

char **jassh_split_line(char *line) 
{
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

char *jassh_read_line(void) 
{
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
