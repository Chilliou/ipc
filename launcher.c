//
// Created by ld201024 on 22/11/22.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> /* for fork */
#include <sys/types.h> /* for pid_t */
#include <sys/wait.h> /* for wait */

int main()
{
    /*Spawn a child to run the program.*/
    pid_t pid=fork();
    if (pid==0) { /* child process */
        static char *argv[]={NULL};
        execv("./ipc",argv);
        exit(127); /* only if execv fails */
    }
    else { /* pid!=0; parent process */
        static char *argv[]={NULL};
        execv("./ipc",argv);
        waitpid(pid,0,0); /* wait for child to exit */

    }
    return 0;
}
