/**  smsh3.c  small-shell version 3
 **		first really useful version after prompting shell
 **		this one parses the command line into strings
 **		uses fork, exec, wait, and ignores signals
 **     added pipeline
 **     added redirection
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "smsh.h"

#define MAXLETTERS 512
#define MAXPARAMS 128

void setup() 
/*
 * purpose: initialize shell
 * returns: nothing. calls fatal() if trouble
 */
{
	signal(SIGINT,  SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
} 

void fatal(char *s1, char *s2, int n)
{
	fprintf(stderr,"Error: %s,%s\n", s1, s2);
	exit(n);
}

int main(int argc, char **argv) {

    char input[MAXLETTERS+1]; //stdin
    char arrcpy[MAXLETTERS+1]; 
    char *args[MAXPARAMS]; // cmd before pipe
    char *argsPiped[MAXPARAMS]; // cmd after pipe 
    int k=0;
    int execFlag = 0; //flag to call correct function

    //initiate shell
    setup();

    int pid;
    char *token;
    char *separator = " \t\n";
    char **redirect;
    char **redirect2;
    char *copyarr;
    char *infile;
    char *outfile;
    int i, j, x, t;
    int Rflag;

    redirect = malloc(128 * sizeof(char *));
    redirect2 = malloc(128 * sizeof(char *));

    while (1) {
        printf("> ");
        fflush(stderr);

        //stdin
        if (fgets(input, 128, stdin) == NULL)
            break;

        if(input[strlen(input)-1] == '\n') { //remove newline char    
            input[strlen(input)-1] = '\0';    
        }

        for (t=0; t < 128; ++t){
            arrcpy[t] = input[t];
        }

        execFlag = processString(input, args, argsPiped); 

        //copy input into new array 
        for (x=0; x < 128; ++x){
            if ((arrcpy[x] == '>') || (arrcpy[x] == '<')){
                execFlag = 3;
            }
        }

        //printf("execFlag: %d\n", execFlag);
        
        if (execFlag == 1){ // execute normal commands
            execArgs(args); 
        }
        else if (execFlag == 2) { // excecute piping
            execPipedArgs(args, argsPiped); 
        }
        else if (execFlag == 3){ // redirection
            i = 0;
            while (1) {
                token = strtok((i == 0) ? arrcpy : NULL, separator);
                if (token == NULL)
                    break;
                //populate tokenized command array
                redirect[i++] = token; 
            }
            
            redirect[i] = NULL;
            if (i == 0)
                continue;

            // assume no redirections
            outfile = NULL;
            infile = NULL;

            // split off the redirections
            j = 0;
            i = 0;
            Rflag = 0;
            while (1) {
                copyarr = redirect[i++];
                if (copyarr == NULL)
                    break;

                switch (*copyarr) {
                // if input 
                case '<':
                    if (copyarr[1] == 0)
                        copyarr = redirect[i++];
                    else
                        ++copyarr;
                    infile = copyarr;
                    if (copyarr == NULL)
                        Rflag = 1;
                    else
                        if (copyarr[0] == 0)
                            Rflag = 1;
                    break;
                // if output 
                case '>':
                    if (copyarr[1] == 0)
                        copyarr = redirect[i++];
                    else
                        ++copyarr;
                    outfile = copyarr;
                    if (copyarr == NULL)
                        Rflag = 1;
                    else
                        if (copyarr[0] == 0)
                            Rflag = 1;
                    break;

                default:
                    redirect2[j++] = copyarr;
                    break;
                }
            }
            redirect2[j] = NULL;

            if (Rflag)
                continue;

            // if no child arguments
            if (j == 0)
                continue;

            switch (pid = fork()) {
            case 0:
                // open stdin
                if (infile != NULL) {
                    int fd = open(infile, O_RDONLY);

                    if (dup2(fd, STDIN_FILENO) == -1) {
                        fprintf(stderr, "dup2 failed");
                    }
                    close(fd);
                }
                // open stdout
                if (outfile != NULL) {
                    // redirect[1] = NULL;
                    int fd2;
                    if ((fd2 = open(outfile, O_WRONLY | O_CREAT, 0644)) < 0) {
                        perror("couldn't open output file.");
                        exit(0);
                    }
                    dup2(fd2, STDOUT_FILENO);
                    close(fd2);
                }
                execvp(redirect2[0], redirect2);// child
                signal(SIGINT, SIG_DFL);
                fprintf(stderr, "ERROR %s no such program\n", arrcpy);
                exit(1);
                break;

            case -1:
                fprintf(stderr, "ERROR can't create child process!\n");
                break;

            default:
                wait(NULL);
                }
            }
        }

    exit(0);
}
