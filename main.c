#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <ctype.h>

#define ARGV_SIZE 8
#define STR(var) #var

typedef enum
{
    TK_ID,
    TK_HEAD,
    TK_PIPE,
    TK_REDIRECT_OUT_NEW,
    TK_REDIRECT_OUT_ADD,
    TK_REDIRECT_IN,
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

void append_token(TokenList* lst, char* str, size_t n, TokenKind kind)
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
            printf("'%s':", cur->str);
            switch (cur->kind)
            {
            case TK_HEAD:             printf("%s", STR(TK_HEAD));               break;
            case TK_ID:               printf("%s", STR(TK_ID));                 break;
            case TK_PIPE:             printf("%s", STR(TK_PIPE));               break;
            case TK_REDIRECT_IN:      printf("%s", STR(TK_REDIRECT_IN));        break;
            case TK_REDIRECT_OUT_ADD: printf("%s", STR(TK_REDIRECT_OUT_ADD));   break;
            case TK_REDIRECT_OUT_NEW: printf("%s", STR(TK_REDIRECT_OUT_NEW));   break;
            default:
                printf("NOT DEFINED");
                break;
            }
            if(cur->next){
                printf(", ");
            }
        }
        printf("]\n");
    }
}

bool is_space(char* c)
{
    return (*c == ' ' || *c == '\t');
}
bool is_pipe(char* c)
{
    return *c == '|';
}

bool is_leftangle_single(char* c)
{
    return *c == '<';
}

bool is_rightangle_single(char* c)
{
    return *c == '>';
}
bool is_rightangle_double(char* c)
{
    return strncmp(c, ">>", 2) == 0;
}

bool is_identify(char* c)
{
    return 0x21 <= *c && *c <= 0x7e;
}

int read_identify(char* str)
{
    char* p = str;
    while(*p && is_identify(p)){ p++;}
    return p-str;
}

TokenList* tokenize(char* cmdline)
{
    TokenList* lst = make_tokenlist();
    char* p = cmdline;
    while(*p)
    {
        if(is_space(p)){
            p++;
            continue;
        }
        if(is_pipe(p)){
            append_token(lst, p, 1, TK_PIPE);
            p++;
            continue;
        }
        if(is_rightangle_double(p)){
            append_token(lst, p, 2, TK_REDIRECT_OUT_ADD);
            p += 2;
            continue;
        }
        if(is_rightangle_single(p)){
            append_token(lst, p, 1, TK_REDIRECT_OUT_NEW);
            p++;
            continue;
        }
        if(is_leftangle_single(p)){
            append_token(lst, p, 1, TK_REDIRECT_IN);
            p++;
            continue;
        }
        if(is_identify(p)){
            int n = read_identify(p);
            append_token(lst, p, n, TK_ID);
            p += n;
            continue;
        }
        fprintf(stderr, "[Token Error] %s\n", p);
        exit(1);
    }
    return lst;
}

typedef enum
{
    CMD_EXEC,
    CMD_REDIRECT_OUT_ADD,
    CMD_REDIRECT_OUT_NEW,
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
    for(int i=0; i<argc; i++) {
        cmd->argv[i] = calloc(1, sizeof(char*)*strlen(argv[i]));
        strncpy(cmd->argv[i], argv[i], strlen(argv[i]));
    }
    cmd->argv[argc] = 0;
    return cmd;
}

void append_command(CommandList* cmdlst, int argc, char** argv, CommandKind kind){
    Command* cur = cmdlst->head;
    while(cur->next){
        cur = cur->next;
    }
    cur->next = make_command(argc, argv, kind);
}

CommandList* parse(TokenList* tknlst)
{
    CommandList* cmdlst = make_commandlist();
    Token* cur = tknlst->head;
    while(cur)
    {
        if(cur->kind == TK_HEAD){
            cur = cur->next;
            continue;
        }
        if(cur->kind == TK_PIPE){
            cur = cur->next;
            continue;
        }
        if(cur->kind == TK_REDIRECT_OUT_ADD){
            cur = cur->next;
            if(!cur){
                fprintf(stderr, "[Syntax Error] '>>' に続くファイル名がありません\n");
                return NULL;
            }
            char** argv = calloc(1, sizeof(char*));
            argv[0] = calloc(1, sizeof(char)*strlen(cur->str));
            strncpy(argv[0], cur->str, strlen(cur->str));
            append_command(cmdlst, 1, argv, CMD_REDIRECT_OUT_ADD);
            cur = cur->next;
            continue;
        }
        if(cur->kind == TK_REDIRECT_OUT_NEW){
            cur = cur->next;
            if(!cur){
                fprintf(stderr, "[Syntax Error] '>' に続くファイル名がありません\n");
                return NULL;
            }
            char** argv = calloc(1, sizeof(char*));
            argv[0] = calloc(1, sizeof(char)*strlen(cur->str));
            strncpy(argv[0], cur->str, strlen(cur->str));
            append_command(cmdlst, 1, argv, CMD_REDIRECT_OUT_NEW);
            cur = cur->next;
            continue;
        }
        if(cur->kind == TK_REDIRECT_IN){
            cur = cur->next;
            if(!cur){
                fprintf(stderr, "[Syntax Error] '<' に続くファイル名がありません\n");
                return NULL;
            }
            char** argv = calloc(1, sizeof(char*));
            argv[0] = calloc(1, sizeof(char)*strlen(cur->str));
            strncpy(argv[0], cur->str, strlen(cur->str));
            append_command(cmdlst, 1, argv, CMD_REDIRECT_IN);
            cur = cur->next;
            continue;
        }
        if(cur->kind == TK_ID){
            int argc = 0;
            int capa = ARGV_SIZE;
            char** argv = calloc(1, sizeof(char*)*capa);
            while(cur && cur->kind == TK_ID){
                if(capa <= argc){
                    capa += ARGV_SIZE;
                    argv = realloc(argv, sizeof(char*)*capa);
                }
                argv[argc] = calloc(1, sizeof(char)*strlen(cur->str));
                strncpy(argv[argc], cur->str, strlen(cur->str));
                argc++;
                cur = cur->next;
            }
            if(capa <= argc){
                capa += 1;
                argv = realloc(argv, sizeof(char*)*capa);
            }
            argv[argc] = 0;
            append_command(cmdlst, argc, argv, CMD_EXEC);
            continue;
        }
        fprintf(stderr, "[Syntax error] '%s'\n", cur->str);
        exit(1);
    }
    return cmdlst;
}

void print_commandlist(CommandList* cmdlst)
{
    if(cmdlst->head == NULL){
        printf("[null]\n");
    }else{
        Command* cur = cmdlst->head;
        while(cur->next){
            cur = cur->next;
            printf("[");
            for(int i=0; i<cur->argc; i++){
                printf("'%s'", cur->argv[i]);
                if(i < cur->argc-1){
                    printf(", ");
                }
            }
            printf("]:");
            switch (cur->kind)
            {
            case CMD_EXEC:          printf("%s", STR(CMD_EXEC));         break;
            case CMD_REDIRECT_IN:   printf("%s", STR(CMD_REDIRECT_IN));  break;
            case CMD_REDIRECT_OUT_ADD:  printf("%s", STR(CMD_REDIRECT_OUT_ADD)); break;
            case CMD_REDIRECT_OUT_NEW:  printf("%s", STR(CMD_REDIRECT_OUT_NEW)); break;
            default:
                printf("NOT DEFINED");
                break;
            }
            if(cur->next){
                printf(" -> ");
            }
        }
        printf("\n");
    }
}


int main(int argc, char* argv[])
{
    // if(argc != 2){
    //     fprintf(stderr, "Argument is short.\n");
    //     exit(EXIT_FAILURE);
    // }
    // TokenList* tknlst = tokenize(argv[1]);
    // print_tokenlist(tknlst);
    // CommandList* cmdlst = parse(tknlst);
    // print_commandlist(cmdlst);
    char cmd[1024];
    while(1)
    {
        printf("$ ");
        fgets(cmd, 1024, stdin);
        char* _c = strchr(cmd, '\n');
        if(_c != NULL){ *_c = '\0';}
        char* p = cmd;
        // char* argv[5];
        // int argc = 0;
        // while(*p) // NULLチェック
        // {
        //     if(*p && 0x21 <= *p && *p <= 0x7e) // 0x21<= <=0x7e: 印字可能文字
        //     {
        //         char* head;
        //         char* end;
        //         head = p; // 印字可能文字の先頭アドレスを覚える
        //         end  = p;
        //         p++;
        //         while(*p && 0x20 <= *p && *p <= 0x7e)
        //         {
        //             if(0x21 <= *p && *p <= 0x7e)
        //             {
        //                 end = p;
        //                 p++;
        //                 continue;
        //             }
        //             if(0x20 == *p)
        //             {
        //                 p++;
        //                 break;
        //             }
        //         }
        //         argv[argc] = (char*)calloc(1, end-head+1);
        //         strncpy(argv[argc], head, end-head+1);
        //         argc++;

        //     }
        // }

        // argv[argc] = 0;
        // for(int i=0; i<argc; i++)
        // {
        //     printf("%s\n", argv[i]);
        // }
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
            // execvp(argv[0], argv);
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
        // for(int i=0; i<argc; i++){
        //     free(argv[i]);
        // }
        memset(cmd, 0, 1024);
    }
    return 0;
}