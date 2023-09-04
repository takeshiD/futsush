# コマンド実行
bashなどのシェルでは既に実装されている`ls`や`gcc`などのプログラム名を打ち込むとそのプログラムを実行することができます。
```sh
$ ls
README.md main.c
$ gcc ./main.c # a.outが出力される
```
あまりにも当たり前すぎて何の疑問も持ちにくいのですが、これはシステムコール`exec`に対してコマンドを表す文字列を渡すことで実行をしており、シェルはその橋渡しをしているだけです。

## プロセスに関わるシステムコール
シェルがコマンドを実行するときに使うシステムコールを紹介しましょう。

### fork
forkをプロセス内で呼び出すと、呼び出した時点でそのプロセスを2つに複製します。  
呼び出し元のプロセスを親プロセス、複製されたプロセスを子プロセスと言います。子プロセスは正常に終了すると親プロセスに対してSIGCHLDというシグナルを送信します。

### exec
execに成功するとその時点で自プロセスは書き換えられます。失敗すると書き換えられずに自プロセスが継続します。

### wait
forkした子プロセスの終了はwaitによって待たなければなりません。
プロセス自体は終了したが、waitによって終了が待たれる機会を失ったプロセスはゾンビと言われます。


### fork-exec
Linuxではプロセスを実行する手法として次の手順が一般的です。

1. forkをする
2. 子プロセスでexecを実行する
3. 親プロセスでwaitして子プロセスの終了を待つ(この間、親プロセスは停止する)
4. 子プロセスが終了し、親プロセスにSIGCHLDを送信する
5. 親プロセスが再開する

これをfork-execと呼びます。
図にすると以下のようになります。

![](/doc/img/fork-exec-wait.svg)

# 1つのコマンドを実行するプログラム
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
$ gcc ./main.c
$ ./a.out ls -l
合計 32
-rw-rw-r-- 1 tkcd tkcd  1061  9月  3 15:42 LICENSE
-rw-rw-r-- 1 tkcd tkcd  3174  9月  4 00:04 README.md
-rwxrwxr-x 1 tkcd tkcd 16312  9月  4 06:52 a.out
drwxrwxr-x 3 tkcd tkcd  4096  9月  3 21:42 doc
-rw-rw-r-- 1 tkcd tkcd   691  9月  4 06:52 main.c
pid=54408 is completed
```

ただし`execlp(argv[0], argv[1], argv[2], NULL)`と、インデックスを2まで指定してしまっているため、引数がついていないコマンドは失敗します。
```sh
$ ./a.out ls
Argument is failed
```

## execvpを使う
execvpのほうが指定が楽なので、差し替えてみましょう。
execvpは`char*[]`を指定するので引数の長さに応じて動的に配列を確保するように変更します。
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
        // cmd[argc-1] = '\0';
        cmd[argc-1] = 0;
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
`cmd[argc-1] = '\0';`で`cmd`配列の終端にNULL文字(='\0')を代入しています。`execvp`はNULLがある時点で終端と判定する関数のためこのような操作を行っています。

> ASCIIコード表で0該当する文字がNULL文字とされており、char型の表現で`\0`となります。gccでは`cmd[artgc-1] = 0;`としても同じです。
> 実はcallocで0に初期化してるので代入する必要は無いのですが説明のため入れてます。あと分かりやすいです。

引数の長さに応じて動的に確保するので、`ls`のみでも実行できます。
```sh
$ gcc ./main.c
$ ./a.out ls
LICENSE  README.md  a.out  doc  main.c
pid=55559 is completed
```