/**  smsh2.c  small-shell version 2
 **		first really useful version after prompting shell
 **		this one parses the command line into strings
 **		uses fork, exec, wait, and ignores signals
 **     add pipeline
 **/

#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <signal.h>
#include "smsh.h"

#define MAXLETTERS 512
#define MAXPARAMS 128

int main(int argc, char **argv) 
{ 
	char input[MAXLETTERS+1]; //stdin
	char *cmd1[MAXPARAMS+1] = {0}; // cmd before pipe
	char *cmd2[MAXPARAMS+1] = {0}; // cmd after pipe 
	int flag = 0; //flag to call correct function

	//initiate shell
	setup(); 

	while (1) { 

		printf("> ");
		if(fgets(input, sizeof(input), stdin) == NULL) break;
		 if(input[strlen(input)-1] == '\n') { //remove newline char    
            input[strlen(input)-1] = '\0';    
        }
		// process input
		//printf("\n%s", input);
		flag = processString(input, cmd1, cmd2); 
		
		// execute 
		if (flag == 1) 
			execArgs(cmd1); 

		if (flag == 2) 
			execPipedArgs(cmd1, cmd2); 
	} 
	return 0; 
} 

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