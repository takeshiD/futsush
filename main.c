#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <ctype.h>

typedef enum
{
    TK_ID,
    TK_HEAD,
} TokenKind;

typedef struct Token Token;
struct Token
{
    char* str;
    TokenKind kind;
    Token* next;
};

typedef struct TokenList TokenList;
struct TokenList
{
    Token* head;
};

TokenList* make_tokenlist()
{
    TokenList* lst = calloc(1, sizeof(TokenList));
    lst->head = calloc(1, sizeof(Token));
    lst->head->kind = TK_HEAD;
    lst->head->next = NULL;
    return lst;
}

Token* make_token(char* str, size_t n, TokenKind kind)
{
    Token* tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = calloc(1, n);
    strncpy(tok->str, str, n);
    return tok;
}

void append(TokenList* lst, char* str, size_t n, TokenKind kind)
{
    Token* cur = lst->head;
    while(cur->next)
    {
        cur = cur->next;
    }
    cur->next = make_token(str, n, kind);
}

bool isempty(TokenList* lst)
{
    return lst->head->next == NULL;
}

void print_tokenlist(TokenList* lst)
{
    if(isempty(lst)){
        printf("[null]\n");
    }else{
        printf("[");
        Token* cur = lst->head;
        while(cur->next){
            cur = cur->next;
            printf("'%s'", cur->str);
            if(cur->next){
                printf(", ");
            }
        }
        printf("]\n");
    }
}
bool is_identify(char c)
{
    return 0x21 <= c && c <= 0x7e;
}

int read_identify(char* str)
{
    char* p = str;
    while(*p && is_identify(*p)){ p++;}
    return p-str;
}

TokenList* tokenize(char* cmdline)
{
    TokenList* lst = make_tokenlist();
    char* p = cmdline;
    while(*p)
    {
        if(isspace(*p)){
            p++;
            continue;
        }
        if(is_identify(*p)){
            int n = read_identify(p);
            append(lst, p, n, TK_ID);
            p += n;
            continue;
        }
    }
    return lst;
}



typedef enum
{
    CMD_EXEC,
    CMD_REDIRECT_OUT,
    CMD_REDIRECT_IN,
    CMD_HEAD,
} CommandKind;

typedef struct Command Command;
struct Command
{
    int argc;
    char** argv;
    CommandKind kind;
    Command* next;
};

typedef struct CommandList CommandList;
struct CommandList
{
    Command* head;
};

CommandList* make_commandlist()
{
    CommandList* lst = calloc(1, sizeof(CommandList));
    lst->head = calloc(1, sizeof(Command));
    lst->head->kind = CMD_HEAD;
    lst->head->next = NULL;
    return lst;
}

Command* make_command(int argc, char** argv, CommandKind kind)
{
    Command* cmd = calloc(1, sizeof(Command));
    cmd->kind = kind;
    cmd->argc = argc;
    cmd->argv = calloc(argc+1, sizeof(char*));
    for(int i=0; i<argc; i++)
    {
        cmd->argv[i] = calloc(1, sizeof(argv[i]));
        strncpy(cmd->argv[i], argv[i], sizeof(argv[i]));
    }
    cmd->argv[argc] = '\0';
    return cmd;
}


int main(int argc, char* argv[])
{
    if(argc != 2){
        fprintf(stderr, "Argument is short.\n");
        exit(EXIT_FAILURE);
    }
    TokenList* lst = tokenize(argv[1]);
    print_tokenlist(lst);
    // char cmd[1024];
    // while(1)
    // {
    //     printf("$ ");
    //     fgets(cmd, 1024, stdin);
    //     char* _c = strchr(cmd, '\n');
    //     if(_c != NULL){ *_c = '\0';}
    //     char* p = cmd;
    //     char* argv[5];
    //     int argc = 0;
    //     while(*p) // NULLチェック
    //     {
    //         if(*p && 0x21 <= *p && *p <= 0x7e) // 0x21<= <=0x7e: 印字可能文字
    //         {
    //             char* head;
    //             char* end;
    //             head = p; // 印字可能文字の先頭アドレスを覚える
    //             end  = p;
    //             p++;
    //             while(*p && 0x20 <= *p && *p <= 0x7e)
    //             {
    //                 if(0x21 <= *p && *p <= 0x7e)
    //                 {
    //                     end = p;
    //                     p++;
    //                     continue;
    //                 }
    //                 if(0x20 == *p)
    //                 {
    //                     p++;
    //                     break;
    //                 }
    //             }
    //             argv[argc] = (char*)calloc(1, end-head+1);
    //             strncpy(argv[argc], head, end-head+1);
    //             argc++;
    //         }
    //     }
    //     argv[argc] = 0;
    //     for(int i=0; i<argc; i++)
    //     {
    //         printf("%s\n", argv[i]);
    //     }
    //     pid_t pid = fork();
    //     if(pid < 0)
    //     {
    //         fprintf(stderr, "Error fork\n");
    //         return EXIT_FAILURE;
    //     }
    //     else if(pid == 0)
    //     { // 子プロセス
    //         execvp(argv[0], argv);
    //         fprintf(stderr, "Error exec\n");
    //         return EXIT_FAILURE;
    //     }
    //     else
    //     { // 親プロセス
    //         int status;
    //         waitpid(pid, &status, 0);
    //         fprintf(stdout, "pid=%d is completed\n", pid);
    //     }
    //     for(int i=0; i<argc; i++){
    //         free(argv[i]);
    //     }
    //     memset(cmd, 0, 1024);
    // }
    return 0;
}