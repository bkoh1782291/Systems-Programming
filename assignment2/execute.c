#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <signal.h>
#include <fcntl.h>
#include <glob.h>
#include "smsh.h"


// Function where the system command is executed 
void execArgs(char** argv1) 
{ 

    if ( argv1[0] == NULL ) 
        return;

    // Forking a child 
    pid_t pid = fork(); 

    if (pid == -1) { 
        perror("first fork fail\n"); 
    } 
    else if (pid == 0) { 
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        if (execvp(argv1[0], argv1) < 0) { 
            printf("Could not execute command\n"); 
        } 
        exit(0); 
    } else { 
        // waiting for child to terminate 
        wait(NULL); 
        return; 
    } 
} 

// Function where the piped system commands is executed 
void execPipedArgs(char** argv1, char** argv2) 
{ 
    // 0 is read end, 1 is write end 
    int pipefd[2]; 
    pid_t p1, p2; 

    if (pipe(pipefd) < 0) { 
        printf("Pipe could not be initialized\n"); 
        return; 
    } 
    //first fork
    p1 = fork(); 
    if (p1 < 0) { 
        printf("Could not fork\n"); 
        return; 
    } 

    if (p1 == 0) { 
        close(pipefd[0]); 
        dup2(pipefd[1], STDOUT_FILENO); 
        close(pipefd[1]); 

        if (execvp(argv1[0], argv1) < 0) { 
            printf("\ncommand 1 failed\n"); 
            exit(0); 
        } 
    } else { 
        // Parent executing 
        p2 = fork(); 

        if (p2 < 0) { 
            printf("\nCould not fork\n"); 
            return; 
        } 

        if (p2 == 0) { 
            close(pipefd[1]); 
            dup2(pipefd[0], STDIN_FILENO); 
            close(pipefd[0]); 
            if (execvp(argv2[0], argv2) < 0) { 
                printf("\ncommand 2 failed\n"); 
                exit(0); 
            } 
        } else { 
            // wait for child process
            // close pipes
            close(pipefd[1]); 
            close(pipefd[0]);
            wait(NULL); 
            wait(NULL); 
        } 
    } 
} 


int LocalCmdHandler(char** parsed) 
{ 
    int NumOfcmds = 4, i, switchOwnArg = 0; 
    char* ListOfOwnCmds[NumOfcmds]; 
    char* username; 

    ListOfOwnCmds[0] = "exit"; 
    ListOfOwnCmds[1] = "cd"; 
    ListOfOwnCmds[2] = "help"; 
    ListOfOwnCmds[3] = "hello"; 

    for (i = 0; i < NumOfcmds; i++) { 
        if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) { 
            switchOwnArg = i + 1; 
            break; 
        } 
    } 

    switch (switchOwnArg) { 
    case 1: 
        exit(0); 
    case 2: 
        chdir(parsed[1]); 
        return 1; 
    case 3: 
        //openHelp(); 
        return 1; 
    case 4: 
        username = getenv("USER"); 
        printf("%s\n", username); 
        return 1; 
    default: 
        break; 
    } 

    return 0; 
} 

void execglob(char *line){

	glob_t globbuf;
	char arrcpy[20+1];
	int t;
	int x;
	char **arr;
	char *token;
	char *separator = " \t\n";
	int params;
	arr = malloc(128 * sizeof(char *));

	if(line[strlen(line)-1] == '\n') { //remove newline char    
        line[strlen(line)-1] = '\0';    
    }

    for (t=0; t < 20; ++t){
        arrcpy[t] = line[t];
    }

    int i = 0;
    while (1) {
        token = strtok((i == 0) ? arrcpy : NULL, separator);
        if (token == NULL)
            break;
        arr[i++] = token;              /* build command array */
    }

    arr[i] = NULL;
    //printf("i : %d\n", i);
    params = i-1;

    //printf("arr1: %s\n", arr[0]);
    //printf("arr2: %s\n", arr[1]);
    //printf("arr3: %s\n", arr[2]);

    // Forking a child 
    pid_t pid = fork(); 

    if (pid == -1){
    	perror("glob fork fail\n");
    }
    else if (pid == 0)
    {
	    globbuf.gl_offs = params;
		glob(arr[params], GLOB_DOOFFS, NULL, &globbuf);
		for (x = 0; x < params; ++x){ 
			globbuf.gl_pathv[x] = arr[x];
		}
		execvp(globbuf.gl_pathv[0], globbuf.gl_pathv);
		printf("\n");
	}
	else {
		wait(NULL);
		return;
	}

	globfree(&globbuf);

}