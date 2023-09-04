#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
int main(int argc, char* argv[])
{
    if(argc != 3){
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
        execlp(argv[1], argv[1], argv[2], NULL);
        fprintf(stderr, "Error exec\n");
        return EXIT_FAILURE;
    }
    else
    { // 親プロセス
        int status;
        waitpid(pid, &status, 0);
        fprintf(stdout, "pid=%d is completed\n", pid);
    }
    return 0;
}