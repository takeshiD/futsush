#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
int main()
{
    char cmd[1024];
    while(1)
    {
        printf("$ ");
        gets(cmd);
        char* p = cmd;
        char* argv[5];
        int argc = 0;
        while(*p) // NULLチェック
        {
            if(*p && 0x21 <= *p && *p <= 0x7e) // 0x21<= <=0x7e: 印字可能文字
            {
                char* head;
                char* end;
                head = p; // 印字可能文字の先頭アドレスを覚える
                end  = p;
                p++;
                while(*p && 0x20 <= *p && *p <= 0x7e)
                {
                    if(0x21 <= *p && *p <= 0x7e)
                    {
                        end = p;
                        p++;
                        continue;
                    }
                    if(0x20 == *p)
                    {
                        p++;
                        break;
                    }
                }
                argv[argc] = calloc(1, end-head+1);
                strncpy(argv[argc], head, end-head+1);
                argc++;
            }
        }
        argv[argc] = '\0';
        // for(int i=0; i<argc; i++)
        // {
        //     printf("%s\n", argv[i]);
        // }
        pid_t pid = fork();
        if(pid < 0)
        {
            fprintf(stderr, "Error fork\n");
            return EXIT_FAILURE;
        }
        else if(pid == 0)
        { // 子プロセス
            execvp(argv[0], argv);
            fprintf(stderr, "Error exec\n");
            return EXIT_FAILURE;
        }
        else
        { // 親プロセス
            int status;
            waitpid(pid, &status, 0);
            fprintf(stdout, "pid=%d is completed\n", pid);
        }
        for(int i=0; i<argc; i++){
            free(argv[i]);
        }
    }
    return 0;
}