<<<<<<< HEAD
# 字句解析、構文解析
前章までで単一コマンドの実行が引数も付きで出来ました。
ただしスペースを上手く入れなければならないなど少し面倒です。

ここまでは単一コマンドだけを考えていたので面倒程度で済みましたが、シェルは複数のコマンドの結果を繋げるパイプライン `|` がありますし、複数行の入力などで行頭を合わせるためにタブ文字を入れる、中括弧`{}`で括る、などもあります。追加機能が欲しいときもあるでしょう。

出来なくは無いですが、前章のようなやり方で進めていると後々修正が大変になってしまいます。

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

デバッグ用にTokenListを表示する関数print_tokenlistも作りました。

実行出来るか確認してみましょう。コマンドライン引数に指定した文字列を字句解析できるはずです。

```sh
$ gcc ./main.c
$ ./a.out "hello world |    asdasd     kkkk"
['hello', 'world', '|', 'asdasd', 'kkkk']
```

スペースがいくつ入っても単語ごとに分割出来ていますね。
これで字句解析の実装は完了しました。

> 今回は空白を無視して単語で区切る程度だったのでスクラッチできました。実はlexやflexというソフトウェアでルールを指定してコードを自動生成することが可能です。


# 構文解析
次はトークン列を構文解析します。
通常でしたらトークン列を構文木という構造に変換していくのですが、今回は実装を簡単にするため構文木にはしません。

パイプ`|`とリダイレクト`<`や`>`、`>>`をコマンド区切りとしたコマンド列を作ることとします。

たとえば以下のようになります。

```sh
ls -la | grep sh > result.txt
["ls", "-la"] -> ["grep", "sh"] -> ["result.txt"]
```

`["ls", "-la"]`、`["grep", "sh"]`をパイプで繋がったコマンドとして受け取り、"result.txt"をリダイレクトの出力先に設定するという形です。
=======
# 字句解析、構文解析
>>>>>>> b77d6d4 (l)
