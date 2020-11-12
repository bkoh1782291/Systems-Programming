#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <signal.h>
#include <fcntl.h>

#define MAXLETTERS 512
#define MAXPARAMS 128
#define size 128

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

// Function where the system command is executed 
void execArgs(char** argv1) 
{ 

    if ( argv1[0] == NULL )      /* nothing succeeds */
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

void execRedir(char** argv1){

    if ( argv1[0] == NULL )      /* nothing succeeds */
    return;

    pid_t pid = fork();
    int status;
    int fd0,fd1,i,in=0,out=0;
    char input[64],output[64];

    if (pid == -1) { 
        perror("first fork fail\n"); 
    }
    else if (pid == 0)
    {

    // finds where '<' or '>' occurs and make that argv[i] = NULL , to ensure that command wont't read that

    while(argv1[i] != NULL){
        if(strcmp(argv1[i],"<")==0){        
            argv1[i]=NULL;
            strcpy(input,argv1[i+1]);
            in=2;           
        }               
        if(strcmp(argv1[i],">")==0){      
            argv1[i]=NULL;
            strcpy(output,argv1[i+1]);
            out=2;
        }   
        i++;      
    }

    //if '<' char was found in string inputted by user
    if(in) {   
        // fdo is file-descriptor
        int fd0;
        if ((fd0 = open(input, O_RDONLY, 0)) < 0) {
            perror("Couldn't open input file");
            exit(0);
        }           
        // dup2() copies content of fdo in input of preceeding file
        dup2(fd0, 0); // STDIN_FILENO here can be replaced by 0 
        close(fd0); // necessary
    }
    //if '>' char was found in string inputted by user 
    if (out){
        int fd1;   
        if ((fd1 = creat(output , 0644)) < 0) {
            perror("Couldn't open the output file");
            exit(0);
        }           
        dup2(fd1, STDOUT_FILENO); // 1 here can be replaced by STDOUT_FILENO
        close(fd1);
    }
    if (!(execvp(argv1[0], argv1) >= 0)) {     // execute the command  
        printf("*** ERROR: exec failed\n");
        exit(1);
    }
      
    }
    else if (pid > 0)
    {     
        printf("fork() failed!\n");
        exit(1);
    }
    else {                               
        while (!(wait(&status) == pid)); // good coding to avoid race_conditions(errors) 
    }
    
}

int main(){

    char input[MAXLETTERS+1]; //stdin
    char *args[MAXPARAMS]; 
    char *argsPiped[MAXPARAMS]; 

    int i=0;
    int k=0;
    int execFlag = 0; 

    while(1){

        printf("> ");
        if(fgets(input, sizeof(input), stdin) == NULL) break;
         if(input[strlen(input)-1] == '\n') { //remove newline char    
            input[strlen(input)-1] = '\0';    
        }

        execFlag = processString(input, 
        args, argsPiped); 

        printf("args: ");
        while(args[k] != NULL){
            printf("%s", args[k]);
            k++;
        }

        printf("\n");

        while(args[i] != NULL)
        {   
            if(strcmp(args[i], ">") == 0)
            {        
                execFlag = 3;
            } 
            if (strcmp(args[i], "<") == 0){
                execFlag = 4;
            }
            i++;
        }  

        // execute 
        if (execFlag == 1){
            execArgs(args); 
        }

        if (execFlag == 2) {
            execPipedArgs(args, argsPiped); 
        }
        
        
        if (execFlag == 3){
            printf("execute output redirect\n");
            execRedir(args); 
        }


        /*
        if (execFlag == 4){
            printf("execute input redirect\n");
            execRedir(args); 
        }
        */

    }
    return 0;
}