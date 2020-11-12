#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <glob.h>

//manual globbing for ls *.c

int main(int argc, char**argv){

   //printf("ls -l *.c\n");
   
   glob_t globbuf;
   /*
   globbuf.gl_offs = 2; //how many parameters in total
   // change to 2 for ls -l

   glob("*.c", GLOB_DOOFFS, NULL, &globbuf);
   glob("/*.c", GLOB_DOOFFS | GLOB_APPEND, NULL, &globbuf);
   globbuf.gl_pathv[0] = "ls";
   globbuf.gl_pathv[1] = "-l";
   execvp(globbuf.gl_pathv[0], globbuf.gl_pathv);
   */

   //printf("\nls *.c\n");

   char *line = malloc(21);

   while (1) {
        if (fgets(line, 20, stdin) == NULL)
            break;
   }

   printf("line :%s\n", line);

   if (strcmp(line, "ls *.c") == 0){
       globbuf.gl_offs = 1;
       glob("*.c", GLOB_DOOFFS, NULL, &globbuf);
       //glob("/*.c", GLOB_DOOFFS | GLOB_APPEND, NULL, &globbuf);
       globbuf.gl_pathv[0] = "ls";
       //globbuf.gl_pathv[1] = "-l";
       execvp(globbuf.gl_pathv[0], globbuf.gl_pathv);
       printf("\n");
   }
   else if (strcmp(line, "cat *.h") == 0){
       globbuf.gl_offs = 1;
       glob("*.h", GLOB_DOOFFS, NULL, &globbuf);
       //glob("/*.c", GLOB_DOOFFS | GLOB_APPEND, NULL, &globbuf);
       globbuf.gl_pathv[0] = "cat";
       //globbuf.gl_pathv[1] = "-l";
       execvp(globbuf.gl_pathv[0], globbuf.gl_pathv);
   }
   else if (strcmp(line, "ls *.txt") == 0){
       globbuf.gl_offs = 1;
       glob("*.txt", GLOB_DOOFFS, NULL, &globbuf);
       //glob("/*.c", GLOB_DOOFFS | GLOB_APPEND, NULL, &globbuf);
       globbuf.gl_pathv[0] = "ls";
       //globbuf.gl_pathv[1] = "-l";
       execvp(globbuf.gl_pathv[0], globbuf.gl_pathv);
   }
   else {
       printf("glob fail\n");
   }



   return 0;
}