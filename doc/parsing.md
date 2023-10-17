# 字句解析、構文解析
前章までで単一コマンドの実行が引数も付きで出来ました。
ただしスペースを上手く入れなければならないなど少し面倒です。

ここまでは単一コマンドだけを考えていたので面倒程度で済みましたが、シェルは複数のコマンドの結果を繋げるパイプライン `|` がありますし、複数行の入力などで行頭を合わせるためにタブ文字を入れる、中括弧`{}`で括る、などもあります。追加機能が欲しいときもあるでしょう。

出来なくは無いですが、前章のようなやり方で進めていると後々修正が大変です。

このような問題に対して字句解析と構文解析という手法がよく使われます。
かなり深い話題なので本書では最低限しか触れません。

技術として一度知っておくと今後あらゆる場面で役に立つことは請け合いですので、気になる方は以下の記事を手を動かして読んでいくことをオススメします。とてもわかり易く丁寧です。

[低レイヤを知りたい人のためのCコンパイラ作成入門](https://www.sigbus.info/compilerbook)

本章ではシェルを実装する上で必要最低限の字句解析と構文解析を実装していきます。

# 字句解析
字句解析とは文字列を単語単位に分割して列にすることです。単語をトークン、文字列をトークン列に変換することをトークナイズと言います。

字句解析を行うためにはここでいう単語、すなわちトークンを自身で定義しなければなりません。前章までの内容においてトークンは

* 識別子(ID): スペースを除いた1文字以上の印字可能文字

です。
例えば`ls -la`という文字列は下図のように字句解析されます。

![](/doc/img/parsing_3.svg)

この図でトークン列は連結リストで表現しています。以下で行う実装も連結リストでトークン列を実装します。

早速ですが「スペースを除いた1文字以上の印字可能文字」をトークンとしてトークン列を生成するコードを以下に示します。

```c
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
    // .....前章までの内容
    return 0;
}
```

前章までの内容をコメントアウトしてます。それを抜きにしても少し長いコードとなってしまいました。
今回連結リストは

* TokenList: トークン列
* Token: トークン要素
* TokenList = [head] -> [Token] -> [Token] -> ...

という構造でheadというダミートークンを入れており、文字列を格納するのはhead以降のTokenです。

またトークンとして識別するために`enum`を作成して
```c
typedef enum
{
    TK_ID,   // 単語
    TK_HEAD, // トークン列のダミーヘッド
} TokenKind;
```
と定義しています。

デバッグ用にTokenListを表示する関数print_tokenlistも作りました。

実行出来るか確認してみましょう。コマンドライン引数に指定した文字列を字句解析できるはずです。

```sh
$ gcc ./main.c
$ ./a.out "hello world |    asdasd     kkkk"
['hello', 'world', '|', 'asdasd', 'kkkk']
```

スペースがいくつ入っても単語ごとに分割出来ていますね。
これで字句解析の実装は完了しました。

> 今回は空白を無視して単語で区切る程度だったのでスクラッチできました。もう少し複雑な構文を解析する場合は人手では大変なので、lexやflexというソフトウェアでルールを指定してコードを自動生成することが可能です。


# 構文解析
次はトークン列を構文解析します。
通常でしたらトークン列を構文木という構造に変換していくのですが、今回は実装を簡単にするため構文木にはしません。

パイプ`|`とリダイレクト`<`や`>`、`>>`をコマンド区切りとしたコマンド列を作ることとします。

たとえば以下のようになります。

```sh
ls -la | grep sh > result.txt
["ls", "-la"] -> ["grep", "sh"] -> ["result.txt"]
```

この例では`["ls", "-la"]`、`["grep", "sh"]`をパイプで繋がったコマンドとして受け取り、"result.txt"をリダイレクトの出力先に設定するという形になります。

まずは構造を考えてみましょう。先の例をもう一度見てみると
```sh
ls -la | grep sh > result.txt
["ls", "-la"] -> ["grep", "sh"] -> ["result.txt"]
```
`|`や`>`を区切りとしてコマンドがあると見ていいでしょう。
ですが"result.txt"はコマンドではなくファイルですので実行出来ません。
したがって区切り文字に応じて次のように考えなければいけません。

* 実行コマンド "|" 実行コマンド
* 実行コマンド ">" 出力先ファイル

区切り文字の前はいずれにしろ実行コマンドなので、簡略化すると次のようになります。

* "|" 実行コマンド
* ">" 出力先ファイル

つまり区切り文字によって次に来るトークン列の種類を決めることができます。

以上を踏まえて実装してみると次のようになります。

```c
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
    if(argc != 2){
        fprintf(stderr, "Argument is short.\n");
        exit(EXIT_FAILURE);
    }
    TokenList* tknlst = tokenize(argv[1]);
    print_tokenlist(tknlst);
    CommandList* cmdlst = parse(tknlst);
    print_commandlist(cmdlst);
    // char cmd[1024];
    // 中略
    // }
    return 0;
}
```
同じように`print_commandlist`を作成して出力を見れるようにしました。
また、マクロ`#define STR(v) #v`によってenum自身を文字列として表示しています。これに伴って`print_tokenlist`でもトークン種別が表示されるようにしました。

早速実行してみましょう。
```sh
$ gcc ./main.c

$ ./a.out "hello world |    asdasd     kkkk"
['hello':TK_ID, 'world':TK_ID, '|':TK_PIPE, 'asdasd':TK_ID, 'kkkk':TK_ID]
['hello', 'world']:CMD_EXEC -> ['asdasd', 'kkkk']:CMD_EXEC

$ ./a.out "ls -la < a.txt | grep sh >> result.txt"
['ls':TK_ID, '-la':TK_ID, '<':TK_REDIRECT_IN, 'a.txt':TK_ID, '|':TK_PIPE, 'grep':TK_ID, 'sh':TK_ID, '>>':TK_REDIRECT_OUT_ADD, 'result.txt':TK_ID]
['ls', '-la']:CMD_EXEC -> ['a.txt']:CMD_REDIRECT_IN -> ['grep', 'sh']:CMD_EXEC -> ['result.txt']:CMD_REDIRECT_OUT_ADD
```

# ループ実行
ここまでで字句解析、構文解析が完成しましたのでこれをループ実行出来るように戻しましょう。
前章までの内容とほぼ同じなので特別解説はしません。実コード例は以下のようになります。
字句解析や構文解析のコードは同じですので、mainの部分だけ示します。

```c
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
```

以下の部分でCommandListの1個目しか呼び出していないので、`ls -la | grep sh`や`seq 5 > result.txt`などパイプやリダイレクトでつなげたコマンドはまだ実行出来ません。単一のコマンド`ls -la`や`seq 5`などが実行出来ます。
```c
else if(pid == 0)
{   // 子プロセス
    // execvp(argv[0], argv);
    execvp(cmdlist->head->next->argv[0], cmdlist->head->next->argv);
```

それでは実行してみます。

```sh
$ gcc ./main.c
$ ./a.out
$ ls
LICENSE  README.md  a.out  doc  main.c
pid=91678 is completed
$ seq 5
1
2
3
4
5
pid=91750 is completed
$ 
```

Read-Eval-Print-Loopの流れが出来ました。