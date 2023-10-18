#include "parser.h"
#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ARGV_SIZE 8
#define STR(var) #var

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
