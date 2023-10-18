#include "lexer.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define STR(var) #var

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

static bool is_space(char* c)
{
    return (*c == ' ' || *c == '\t');
}
static bool is_pipe(char* c)
{
    return *c == '|';
}

static bool is_leftangle_single(char* c)
{
    return *c == '<';
}

static bool is_rightangle_single(char* c)
{
    return *c == '>';
}
static bool is_rightangle_double(char* c)
{
    return strncmp(c, ">>", 2) == 0;
}

static bool is_identify(char* c)
{
    return 0x21 <= *c && *c <= 0x7e;
}

static int read_identify(char* str)
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