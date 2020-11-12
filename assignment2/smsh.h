#define	YES	1
#define	NO	0

void setup();

int takeInput(char*);
void printDir();
void execArgs(char**);
void execPipedArgs(char**, char**);
void execglob(char* );
int LocalCmdHandler(char**);
int findPipe(char*, char**);
void parseSpace(char*, char**);
int processString(char*, char**, char**);