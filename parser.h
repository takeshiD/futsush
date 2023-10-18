#ifndef PARSER_H
#define PARSER_H
#include "lexer.h"
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

CommandList* make_commandlist();
Command* make_command(int argc, char** argv, CommandKind kind);
void append_command(CommandList* cmdlst, int argc, char** argv, CommandKind kind);
CommandList* parse(TokenList* tknlst);
void print_commandlist(CommandList* cmdlst);
#endif // PARSER_H