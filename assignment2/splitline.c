#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include "smsh.h"

#define MAXLETTERS 512 // max number of letters to be supported 
#define MAXPARAMS 128 // max number of commands to be supported 

// function for finding pipe and parsing commands
int findPipe(char* str, char** strpiped) 
{ 
	int i; 
	for (i = 0; i < 2; i++) { 
		strpiped[i] = strsep(&str, "|"); 
		if (strpiped[i] == NULL) 
			break; 
	} 

	if (strpiped[1] == NULL) 
		return 0; // returns zero if no pipe is found
	else { 
		return 1; 
	} 
} 

// function for parsing commands without pipe
// seperated by space
void parseSpace(char* str, char** parsed) 
{ 
	int i; 

	for (i = 0; i < MAXPARAMS; i++) { 
		parsed[i] = strsep(&str, " ");  //string seperate function

		if (parsed[i] == NULL) 
			break; 
		if (strlen(parsed[i]) == 0) 
			i--; 
	} 
} 

// call methods
int processString(char* str, char** parsed, char** parsedpipe) 
{ 

	char* strpiped[2]; 
	int piped = 0; 

	piped = findPipe(str, strpiped); 

	if (piped) { 
		parseSpace(strpiped[0], parsed); 
		parseSpace(strpiped[1], parsedpipe); 

	} else { 

		parseSpace(str, parsed); 
	} 

	if (LocalCmdHandler(parsed)) 
		return 0; 
	else
		return 1 + piped; 
} 