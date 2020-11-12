/**  smsh1.c  small-shell version 4
 **		first really useful version after prompting shell
 **		this one parses the command line into strings
 **		uses fork, exec, wait, and ignores signals
 **     added pipeline
 **     added redirection
 **     added globbing
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
    char *args[MAXPARAMS]; 
    char *argsPiped[MAXPARAMS]; 
    int k=0;
    int execFlag = 0; 

    setup();

    int pid;
    char *token;
    char *separator = " \t\n";
    char **redirect;
    char **redirect2;
    char *cp;
    char *infile;
    char *outfile;
    int i;
    int j;
    int x;
    int t;
    int err;

    redirect = malloc(128 * sizeof(char *));
    redirect2 = malloc(128 * sizeof(char *));

    while (1) {
        printf("> ");
        fflush(stderr);

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
        // check for globbing
        for (x=0; x < 128; ++x){
            if ((arrcpy[x] == '*')){
                execFlag = 3;
                break;
            }
        }
        //copy input into new array 
        // check for redirection
        for (x=0; x < 128; ++x){
            if ((arrcpy[x] == '>') || (arrcpy[x] == '<')){
                execFlag = 4;
            }
        }

        //printf("execFlag: %d\n", execFlag);
        
        if (execFlag == 1){ // execute normal cmds
            execArgs(args); 
        }
        else if (execFlag == 2) { // execute pipeline
            execPipedArgs(args, argsPiped); 
        }
        else if (execFlag == 3){ // execute globbing
            execglob(arrcpy);
        }
        else if (execFlag == 4){ // redirection
            i = 0;
            while (1) {
                token = strtok((i == 0) ? arrcpy : NULL, separator);
                if (token == NULL)
                    break;
                redirect[i++] = token;              /* build command array */
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
            err = 0;
            while (1) {
                cp = redirect[i++];
                if (cp == NULL)
                    break;

                switch (*cp) {
                case '<':
                    if (cp[1] == 0)
                        cp = redirect[i++];
                    else
                        ++cp;
                    infile = cp;
                    if (cp == NULL)
                        err = 1;
                    else
                        if (cp[0] == 0)
                            err = 1;
                    break;

                case '>':
                    if (cp[1] == 0)
                        cp = redirect[i++];
                    else
                        ++cp;
                    outfile = cp;
                    if (cp == NULL)
                        err = 1;
                    else
                        if (cp[0] == 0)
                            err = 1;
                    break;

                default:
                    redirect2[j++] = cp;
                    break;
                }
            }
            redirect2[j] = NULL;

            // we got something like "cat <"
            if (err)
                continue;

            // no child arguments
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

                // trying to get this to work
                // NOTE: now it works :-)
                // open stdout
                if (outfile != NULL) {
                    // redirect[1] = NULL;
                    int fd2;

                    //printf("PLEASE WORK");
                    if ((fd2 = open(outfile, O_WRONLY | O_CREAT, 0644)) < 0) {
                        perror("couldn't open output file.");
                        exit(0);
                    }

                    // redirect+=2;
                    printf("okay");
                    dup2(fd2, STDOUT_FILENO);
                    close(fd2);
                }

                execvp(redirect2[0], redirect2);        /* child */
                signal(SIGINT, SIG_DFL);
                fprintf(stderr, "ERROR %s no such program\n", arrcpy);
                exit(1);
                break;

            case -1:
                /* unlikely but possible if hit a limit */
                fprintf(stderr, "ERROR can't create child process!\n");
                break;

            default:
                //printf("am I here");
                wait(NULL);
                //waitpid(pid, 0, 0);
                }
            }
        }

    exit(0);
}
