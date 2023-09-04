#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
int main(int argc, char* argv[])
{
    if(argc < 2){
        fprintf(stderr, "Argument is failed.\n");
        exit(1);
    }
    pid_t pid = fork();
    if(pid < 0)
    {
        fprintf(stderr, "Error fork\n");
        return EXIT_FAILURE;
    }
    else if(pid == 0)
    { // 子プロセス
        char** cmd = calloc(argc, sizeof(char*));
        for(int i=0; i<argc-1; i++){
            cmd[i] = (char*)calloc(1, 128);
            strcpy(cmd[i], argv[i+1]);
        }
        cmd[argc-1] = '\0';
        execvp(cmd[0], cmd);
        fprintf(stderr, "Error exec\n");
        return EXIT_FAILURE;
    }
    else
    { // 親プロセス
        int status;
        // waitpid(1000, &status, 0);
        fprintf(stdout, "pid=%d is completed\n", pid);
    }
    while(1){}
    return 0;
}