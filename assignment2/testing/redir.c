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

char *prompt = "% ";

int main()
{
    int pid;
    //int child_pid;
    char line[81];
    char *token;
    char *separator = " \t\n";
    char **args;
    char **args2;
    char *cp;
    char *ifile;
    char *ofile;
    int i;
    int j;
    int err;
    //int check;
    //char history[90];
    //typedef void (*sighandler_t) (int);

    args = malloc(80 * sizeof(char *));
    args2 = malloc(80 * sizeof(char *));

    //signal(SIGINT, SIG_IGN);

    while (1) {
        fprintf(stderr, "%s", prompt);
        fflush(stderr);

        if (fgets(line, 80, stdin) == NULL)
            break;

        // split up the line
        i = 0;
        while (1) {
            token = strtok((i == 0) ? line : NULL, separator);
            if (token == NULL)
                break;
            args[i++] = token;              /* build command array */
        }
        args[i] = NULL;
        if (i == 0)
            continue;

        // assume no redirections
        ofile = NULL;
        ifile = NULL;

        // split off the redirections
        j = 0;
        i = 0;
        err = 0;
        while (1) {
            cp = args[i++];
            if (cp == NULL)
                break;

            switch (*cp) {
            case '<':
                if (cp[1] == 0)
                    cp = args[i++];
                else
                    ++cp;
                ifile = cp;
                if (cp == NULL)
                    err = 1;
                else
                    if (cp[0] == 0)
                        err = 1;
                break;

            case '>':
                if (cp[1] == 0)
                    cp = args[i++];
                else
                    ++cp;
                ofile = cp;
                if (cp == NULL)
                    err = 1;
                else
                    if (cp[0] == 0)
                        err = 1;
                break;

            default:
                args2[j++] = cp;
                break;
            }
        }
        args2[j] = NULL;

        // we got something like "cat <"
        if (err)
            continue;

        // no child arguments
        if (j == 0)
            continue;

        switch (pid = fork()) {
        case 0:
            // open stdin
            if (ifile != NULL) {
                int fd = open(ifile, O_RDONLY);

                if (dup2(fd, STDIN_FILENO) == -1) {
                    fprintf(stderr, "dup2 failed");
                }

                close(fd);
            }

            // trying to get this to work
            // NOTE: now it works :-)
            // open stdout
            if (ofile != NULL) {
                // args[1] = NULL;
                int fd2;

                //printf("PLEASE WORK");
                if ((fd2 = open(ofile, O_WRONLY | O_CREAT, 0644)) < 0) {
                    perror("couldn't open output file.");
                    exit(0);
                }

                // args+=2;
                printf("okay");
                dup2(fd2, STDOUT_FILENO);
                close(fd2);
            }

            execvp(args2[0], args2);        /* child */
            signal(SIGINT, SIG_DFL);
            fprintf(stderr, "ERROR %s no such program\n", line);
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

    exit(0);
}