# コマンド実行
bashなどのシェルでは`ls`や`gcc`など、環境変数`PATH`に含まれるパス先にあるプログラム名を打ち込むと実行することができます。このように既にコンパイルされて保存されているプログラムを外部コマンドなどと言ったりします。

```sh
$ ls
README.md main.c
$ gcc ./main.c # a.outが出力される
```

あまりにも当たり前すぎて疑問も持ちにくいのですが、実はプログラムの実行をするためにシステムコール`exec`を使用しています。
`exec`に対して`ls`や`gcc`、そしてその引数を表す文字列を渡すことで実行をしており、シェルはその橋渡しをしているだけなのです。

本章では単一のコマンドを実行することを目標にしていきます。

# プロセスに関わるシステムコール
シェルがコマンドを実行するときに使うシステムコールを紹介しましょう。

## fork
forkをプロセス内で呼び出すと、呼び出した時点でそのプロセスを2つに複製します。  
呼び出し元のプロセスを親プロセス、複製されたプロセスを子プロセスと言います。子プロセスは正常に終了すると親プロセスに対してSIGCHLDというシグナルを送信します。

## exec
execに成功するとその時点で自プロセスは書き換えられます。失敗すると書き換えられずに自プロセスが継続します。

## wait
forkした子プロセスの終了はwaitによって待たなければなりません。
プロセス自体は終了したが、waitによって終了が待たれる機会を失ったプロセスはゾンビと言われます。


## fork-exec
Linuxではプロセスを実行する手法として次の手順が一般的です。

1. forkをする
2. 子プロセスでexecを実行する
3. 親プロセスでwaitして子プロセスの終了を待つ(この間、親プロセスは停止する)
4. 子プロセスが終了し、親プロセスにSIGCHLDを送信する
5. 親プロセスが再開する

これをfork-execと呼びます。
図にすると以下のようになります。

![](/doc/img/fork-exec-wait.svg)

# 単一のコマンドを実行するプログラム
シェルの第1歩として、コマンドライン引数に指定した文字列をコマンドとして実行するプログラムを作ってみましょう。

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
int main(int argc, char* argv[])
{
    if(argc != 3){
        fprintf(stderr, "Argument is failed.\n");
        exit(1);
    }
    pid_t pid = fork();
    if(pid < 0)
    {
        fprintf(stderr, "Error fork\n");
        return EXIT_FAILURE;
    }
    else if(pid == 0)
    { // 子プロセス
        execlp(argv[1], argv[1], argv[2], NULL);
        fprintf(stderr, "Error exec\n");
        return EXIT_FAILURE;
    }
    else
    { // 親プロセス
        int status;
        waitpid(pid, &status, 0);
        fprintf(stdout, "pid=%d is completed\n", pid);
    }
    return 0;
}
```

上記のコードを`main.c`に保存してコンパイルします。
試しに引数に`ls -l`を指定して実行してみます。

```sh
$ gcc ./main.c  # コンパイル
$ ./a.out ls -l
合計 32
-rw-rw-r-- 1 tkcd tkcd  1061  9月  3 15:42 LICENSE
-rw-rw-r-- 1 tkcd tkcd  3174  9月  4 00:04 README.md
-rwxrwxr-x 1 tkcd tkcd 16312  9月  4 06:52 a.out
drwxrwxr-x 3 tkcd tkcd  4096  9月  3 21:42 doc
-rw-rw-r-- 1 tkcd tkcd   691  9月  4 06:52 main.c
pid=54408 is completed
```

ただし`execlp(argv[1], argv[1], argv[2], NULL)`と、インデックスを2まで指定してしているため、引数がついていないコマンドは失敗します。
```sh
$ ./a.out ls
Argument is failed
```

## execvpで引数指定を柔軟にする
execvpのほうが指定が楽なので差し替えてみましょう。
execvpはcharの二次元配列`char*[]`、つまり
```c
{ "ls", "-la", '\0' }
```
のような文字列の配列を引数に取りますので、動的に配列を確保するように変更します。またexecvpに渡す配列は終端が'\0'(=NULL)である必要があります。

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
int main(int argc, char* argv[])
{
    if(argc < 2){
        fprintf(stderr, "Argument is failed.\n");
        exit(1);
    }
    pid_t pid = fork();
    if(pid < 0)
    {
        fprintf(stderr, "Error fork\n");
        return EXIT_FAILURE;
    }
    else if(pid == 0)
    { // 子プロセス
        char** cmd = calloc(argc, sizeof(char*));
        for(int i=0; i<argc-1; i++){
            cmd[i] = (char*)calloc(1, 128);
            strcpy(cmd[i], argv[i+1]);
        }
        cmd[argc-1] = '\0';
        execvp(cmd[0], cmd);
        fprintf(stderr, "Error exec\n");
        return EXIT_FAILURE;
    }
    else
    { // 親プロセス
        int status;
        waitpid(pid, &status, 0);
        fprintf(stdout, "pid=%d is completed\n", pid);
    }
    return 0;
}
```

`calloc(size_t n, size_t size)`はsize[byte]の領域をn個確保し、0で初期化する関数です。ちなみに`malloc`は初期化をしません。  

`cmd[argc-1] = '\0';`で`cmd`配列の終端にNULL文字(='\0')を代入しています。最終的な指定する配列は以下のようになります。

```c
{"ls", "-la", '\0'}
```

> ASCIIコード表でNULL文字が定義されており、数字で0に割り当てられています。数字では0ですがchar型の表現で`\0`です。gccでは`cmd[artgc-1] = 0;`としても同じです。  
> 実はcallocで0に初期化してるので代入する必要は無いのですが説明のため入れてます。

引数の長さに応じて動的に確保するので、`ls`のみでも実行できます。
```sh
$ gcc ./main.c
$ ./a.out ls
LICENSE  README.md  a.out  doc  main.c
pid=55559 is completed
```

## Appendix ゾンビ
先程のコードを少し修正してゾンビプロセスを体験してみましょう。

```diff
- 34行 waitpid(pid, &status, 0);
+ 34行 // waitpid(pid, &status, 0);
+ 37行 while(1){}
```
上記のように修正してコンパイルして実行します。`while(1)`の無限ループなので止まりません。
```sh
$ ./a.out ls

```
別の端末を立ち上げて`ps f`を実行してみると以下のように`<defunct>`と表示されるプロセスが見えます。これがゾンビプロセスです。
```sh
  57390 pts/1    R+     0:03  \_ ./a.out ls
  57391 pts/1    Z+     0:00      \_ [ls] <defunct> 
```
ゾンビプロセスとはいえ、親プロセスのa.outに結びついているのでa.outが終了すれば終了します。
元の端末に戻ってCtrl-Cで終了しましょう。

# 実行をループする
シェルはインタラクティブなものですので、コマンド実行をループにしてみましょう。


# ループ実行への改造
ここまでは`a.out`のコマンドライン引数に指定するものでしたが、実際のシェルは対話的にプロンプトを表示、コマンド入力を待つ、コマンド実行、実行したらまたプロンプトというループになっています。
このような対話的な形式をREPL(Read-Eval-Print-Loop)と言います。


まずはそれっぽい形になるように

1. ループ開始
2. プロンプト表示("$ "だけ)
3. コマンド受付(char[])
4. コマンド解析(char[] → char*[])
5. コマンド実行
6. 1に戻る

という形で実装してみましょう。

1と6は`while`、5.はここまでの内容を転用できるでしょう。
2.プロンプト表示、3.コマンド受付、4.コマンド解析を考えてみます。


## プロンプト表示とコマンド受付
2.プロンプト表示は`printf`、3.コマンド受付は`gets`でよさそうです。
`gets`で受け取る文字列はとりあえず余裕を見て1024文字まで受け取れるようにしておきます。

```c
char cmd[1024];
while(1)
{
    printf("$ ");
    gets(cmd);
    printf("%s\n", cmd);
}
```

確認のため受け取った文字列を表示するようにしています。
実行すると以下のようになります。Ctrl-Cで終了できます。

```sh
$ ls
ls
$ aaaa
aaaa
$ sasas
sasas
$ ^C
```

> `gets`は非推奨の関数ですのでコンパイル時に`fgets`を使用するよう促すメッセージが出るかもしれません。

## コマンド解析
`gets`で受け取った文字列をコマンドと引数に分けて`execvp`に渡せる形に整形しましょう。
問題を簡単にするために約束事を決めておきます。

1. コマンドと引数は半角スペース1つで区切られる
2. 先頭と終端はスペースが入らない

次のような形だけ受け付けるようにします。

```sh
$ ls -la    # OK
$  ls  -la  # NG
```

では以下に実装例を記載します。

```c
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
        for(int i=0; i<argc; i++)
        {
            printf("%s\n", argv[i]);
        }
        for(int i=0; i<argc; i++){
            free(argv[i]);
        }
    }
    return 0;
}
```

急にややこしくなりましたが内容を説明していきます。



```c
char* p = cmd;
char* argv[5];
int argc = 0;
while(*p)
{
```

![](/doc/img/parsing_1.svg)


`char* p`はポインタで、`cmd`の先頭アドレスを指すように指定しています。
execvpへ渡す二次元配列としてargvを、引数の個数を表すためにargcを用意しました。
`cmd`を指すポインタ`p`の指す値`*p`をNULL(=0)チェックしNULLで無ければwhileループに入ります。



```c
if(*p && 0x21 <= *p && *p <= 0x7e) // 0x21<= <=0x7e: 印字可能文字
{
    char* head; // argvへ格納する文字列の開始アドレス
    char* end;  // argvへ格納する文字列の終了アドレス
    head = p;
    end  = p;
    p++;
```

if文でNULLチェックかつ印字可能文字であるかを確認します。印字可能文字はASCIIコード表で検索してみてください。

cmdは一連の文字列ですので、部分的にargvへスペースで区切ってコピーしなければなりません。

このためにcmdのコマンドを表す文字の開始と終了を指すアドレスをheadとendに保存します。




```c
    while(*p && 0x20 <= *p && *p <= 0x7e)
    {
        if(0x21 <= *p && *p <= 0x7e) # ①コマンド文字列ならループ継続
        {
            end = p;
            p++;
            continue;
        }
        if(0x20 == *p) # ②半角スペースでループ終了
        {
            p++;
            break;
        }
    }
    argv[argc] = calloc(1, end-head+1);
    strncpy(argv[argc], head, end-head+1);
    argc++;
}
```
![](/doc/img/parsing_2.svg)

①で連続した文字であるかの判定です。endのアドレスを更新してpを次の文字列を指すように更新しcontinueします。

②半角スペースがあったらループを終了します。
ループを抜けたらargvの領域を確保し、strncpyでcmdの部分文字列をコピーします。




説明は以上です。ではコンパイルして実行してみましょう。
規定されていないところに半角スペースが入ると無限ループに入るので都度Ctrl-Cで終了してください。

正しくコマンドを入力出来ている場合はargvに格納された文字列が表示されます。
```sh
$ ls -la aaaaa
ls
-la
aaaaa
```

あとはこれをforkしてexecvpに指定するだけです。
この章での完全なコードを以下に示します。

```c
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
```

コンパイルして実行すると以下のような結果となります。

```sh
$ ls -la      
total 52
drwxr-xr-x 4 tkcd tkcd  4096 Sep 10 12:09 .
drwxr-x--- 7 tkcd tkcd  4096 Sep  9 22:34 ..
drwxr-xr-x 8 tkcd tkcd  4096 Sep  9 12:23 .git
-rw-r--r-- 1 tkcd tkcd   279 Sep  9 09:51 .gitignore
-rw-r--r-- 1 tkcd tkcd  1061 Sep  9 09:51 LICENSE
-rw-r--r-- 1 tkcd tkcd  3406 Sep  9 12:23 README.md
-rwxr-xr-x 1 tkcd tkcd 16464 Sep 10 12:09 a.out
drwxr-xr-x 3 tkcd tkcd  4096 Sep  9 12:14 doc
-rw-r--r-- 1 tkcd tkcd  1907 Sep 10 12:09 main.c
pid=289519 is completed
$ seq 5
1
2
3
4
5
pid=289613 is completed
$ 
```