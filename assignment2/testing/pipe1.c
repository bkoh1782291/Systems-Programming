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

int executecmd(char** params) {
    pid_t pid = fork(); //fork process

    if (pid == -1) { //error
        char *error = strerror(errno);
        printf("error fork!!\n");
        return 1;
    } else if (pid == 0) { // child process
        execvp(params[0], params); //exec cmd
        char *error = strerror(errno);
        printf("unknown command\n");
        return 0;
    } else { // parent process
        int childstatus;
        waitpid(pid, &childstatus, 0);    
        return 1;
    }
};

int execpipe (char ** argv1, char ** argv2) {
    
    int fds[2];
    pipe(fds);
    int child_info = -1;
    //int i;

    pid_t pid2 = fork();
    if (pid2 < 0){
        perror("fork fail\n");
    }
    else if (pid2 == 0){
        //second fork
        pid_t pid3 = fork();

        if (pid3 == -1) { //error
            perror("fork fail");
            return 1;
        } 
        if (pid3 == 0) { // child process
            dup2(fds[0], 0);
            close(fds[1]);
            //close(fds[0]);
            execvp(argv2[0], argv2); // run command AFTER pipe character in userinput
            perror("cmd after pipe");
            exit(0);
        } else { // parent process
            dup2(fds[1], 1);
            close(fds[0]);
            //close(fds[1]);
            execvp(argv1[0], argv1); // run command BEFORE pipe character in userinput
            perror("cmd before pipe");
            exit(1);
        }
    }
    else {
        int childstatus;
        waitpid(pid2, &childstatus, 0);
        exit(1);
    }
    return child_info;
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
        int j=parsecmd(cmd, params); //split cmd into array of params           
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
            if (execpipe(argv1, argv2) == 0) break;    
         } 
         else if (f==1) {     
             if (executecmd(params) == 0) break;
         }
    } // end while
    return 0;
}