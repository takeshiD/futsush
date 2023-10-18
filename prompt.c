#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include "prompt.h"
#include "lexer.h"
#include "parser.h"
int prompt(char* ps)
{
    char cmd[1024];
    while(1)
    {
        printf("%s", ps);
        fgets(cmd, 1024, stdin);
        char* _c = strchr(cmd, '\n');
        if(_c != NULL){ *_c = '\0';}
        TokenList* tknlist = tokenize(cmd);
        CommandList* cmdlist = parse(tknlist);
        pid_t pid = fork();
        if(pid < 0)
        {
            fprintf(stderr, "Error fork\n");
            return EXIT_FAILURE;
        }
        else if(pid == 0)
        { // 子プロセス
            execvp(cmdlist->head->next->argv[0], cmdlist->head->next->argv);
            fprintf(stderr, "Error exec\n");
            return EXIT_FAILURE;
        }
        else
        { // 親プロセス
            int status;
            waitpid(pid, &status, 0);
            fprintf(stdout, "pid=%d is completed\n", pid);
        }
        memset(cmd, 0, 1024);
    }
}