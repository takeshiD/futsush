#ifndef LEXER_H
#define LEXER_H
#include <sys/types.h>
#include <stdbool.h>
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

TokenList* make_tokenlist();
Token* make_token();
void append_token(TokenList* lst, char* str, size_t n, TokenKind kind);
bool isempty(TokenList* lst);
void print_tokenlist(TokenList* lst);
TokenList* tokenize(char* cmdline);

#endif // LEXER_H