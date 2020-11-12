#include <stdlib.h>   
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_CMD_LENGTH 100

#define MAX_NUM_PARAMS 10

int parsecmd(char* cmd, char** params) { //split cmd into array of params
    int i,n=-1;
    for(i=0; i<MAX_NUM_PARAMS; i++) {
        params[i] = strsep(&cmd, " ");
        n++;
        if(params[i] == NULL) break;
    }
    return(n);
};

// Function where the system command is executed 
void executecmd(char** parsed) 
{ 
    // Forking a child 
    pid_t pid = fork(); 

    if (pid == -1) { 
        printf("\nFailed forking child.."); 
        return; 
    } else if (pid == 0) { 
        if (execvp(parsed[0], parsed) < 0) { 
            printf("\nCould not execute command.."); 
        } 
        exit(0); 
    } else { 
        // waiting for child to terminate 
        wait(NULL); 
        return; 
    } 
}

void execpipe(char** argv1, char** argv2) 
{ 
    // 0 is read end, 1 is write end 
    int pipefd[2];  
    pid_t p1, p2; 
  
    if (pipe(pipefd) < 0) { 
        printf("\nPipe could not be initialized"); 
        return; 
    } 
    p1 = fork(); 
    if (p1 < 0) { 
        printf("\nCould not fork"); 
        return; 
    } 
  
    if (p1 == 0) { 
        // Child 1 executing.. 
        // It only needs to write at the write end 
        close(pipefd[0]); 
        dup2(pipefd[1], STDOUT_FILENO); 
        close(pipefd[1]); 
  
        if (execvp(argv1[0], argv1) < 0) { 
            printf("\nCould not execute command 1.."); 
            exit(0); 
        } 
    } else { 
        // Parent executing 
        p2 = fork(); 
  
        if (p2 < 0) { 
            printf("\nCould not fork"); 
            return; 
        } 
  
        // Child 2 executing.. 
        // It only needs to read at the read end 
        if (p2 == 0) { 
            close(pipefd[1]); 
            dup2(pipefd[0], STDIN_FILENO); 
            close(pipefd[0]); 
            if (execvp(argv2[0], argv2) < 0) { 
                printf("\nCould not execute command 2.."); 
                exit(0); 
            } 
        } else { 
            // parent executing, waiting for two children 
            close(pipefd[0]);
            close(pipefd[1]); 
            wait(NULL); 
            wait(NULL); 
        } 
    } 
} 


int main() {    
    char cmd[MAX_CMD_LENGTH+1];    
    char * params[MAX_NUM_PARAMS+1];    
    char * argv1[MAX_NUM_PARAMS+1] = {0};    
    char * argv2[MAX_NUM_PARAMS+1] = {0};    
    int k, y, x;    
    int f = 1;    
    while(1) {
        printf("> "); //prompt    
        if(fgets(cmd, sizeof(cmd), stdin) == NULL) break; //read command, ctrl+D exit       
        if(cmd[strlen(cmd)-1] == '\n') { //remove newline char    
            cmd[strlen(cmd)-1] = '\0';    
        }    
        int j = parsecmd(cmd, params); //split cmd into array of params           
        if (strcmp(params[0], "exit") == 0) break; //exit   
        for (k=0; k <j; k++) {  
            if (strcmp(params[k], "|") == 0) {    
                f = 0; y = k;      
               //printf("pipe found\n");
               break;
            }               
        }
        if (f==0) {
            for (x=0; x<k; x++) {    
               argv1[x]=params[x];
            }     
            int z = 0;     
            for (x=k+1; x< j; x++) {     
                argv2[z]=params[x];
                z++;
            }     
            //execpipe(argv1, argv2);
            execpipe(argv1, argv2);
         } 
         else if (f==1) {     
            executecmd(params);
         }

         free(params);
         free(cmd);
    } // end while
    return 0;
}