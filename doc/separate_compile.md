# 分割コンパイル
ここまででそれなりの行数のコードとなりました。
実装していくと分かると思いますが字句解析、構文解析、ループ実行は機能的には独立しています。

後々整理がしやすいように、それぞれでソースファイルを分けましょう。
ソースファイルをいくつかの機能に分けて、最後に一つの実行ファイルにまとめあげることを**分割コンパイル**といいます。

分割コンパイルはテストがしやすくなったり、コンパイル時間が短縮されたりなど恩恵が多いのでオススメです。

# 機能の分割
まずは機能を分割しましょう。ここまでで実装した機能は

* 字句解析
* 構文解析
* ループ実行

の3種です。加えてC言語の場合はmain関数を記述するファイルが必要です。これらをそれぞれ

* 字句解析: lexer.c, lexer.h
* 構文解析: parser.c, parser.h
* ループ実行: prompt.c, prompt.h
* main関数: main.c

として分けましょう。
`*.h`はヘッダファイルと呼ばれ、例えば`prompt.c`から`parser.c`の関数を呼び出したいときに代わりに`parser.h`を呼び出してどのような関数があるか把握するために使います。


# Makefile
ファイルをいくつかに分けたところでコンパイル出来るようにMakefileを作成しましょう。

内容は以下の通りです。

```Makefile
TARGET := futsush
CC := gcc
SRCS := main.c prompt.c parser.c lexer.c
OBJS := $(SRCS:%.c=%.o)
CFLAGS := -std=c11 -Wall -g
INCS := -I.
LIBS := 

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -rf $(TARGET) $(OBJS)
```

`TARGET`: 最終的な実行ファイル名
`CC`: Cコンパイラ
`SRCS`: ソースファイル
`OBJS`: オブジェクトファイル
`HEADERS`: ヘッダファイル
`CFLAGS`: Cコンパイラのオプション
`INCS`: インクルードパス
`LIBS`: ライブラリパス

`OBJS`の`$(SRCS:%.c=%.o)`とは`.c`のファイルを`.o`へ置換する記述です。
上記のMakefileと同ディレクトリ上で`make`を実行すればそれぞれのオブジェクトファイル`.o`がコンパイルされ、最後に`$(TARGET): $(OBJS)`が実行され`futsush`という実行ファイルが生成されます。

では実行してみましょう。

```sh
$ make
gcc -std=c11 -Wall -g   -c -o main.o main.c
gcc -std=c11 -Wall -g   -c -o prompt.o prompt.c
gcc -std=c11 -Wall -g   -c -o parser.o parser.c
gcc -std=c11 -Wall -g   -c -o lexer.o lexer.c
gcc -std=c11 -Wall -g -o futsush main.o prompt.o parser.o lexer.o
```

最後の行は`$(TARGET): $(OBJS)`で定義した内容が実行されているのが分かりますが、それ以外のオブジェクトファイルにコンパイルする記述はMakefileに書いていません。これは**サフィックスルール**と呼ばれるルールがmakeコマンド内に定義されており、`.o`のファイルが必要になったらデフォルトで
```Makefile
.c.o:
    $(CC) $(CFLAGS) -c -o $< $@
```
が実行されるように定義されています。
Makefile内で`.c.o`を新たに定義すれば上書きされます。


`clean`というコマンドを定義していますので、生成されたオブジェクトファイルと実行ファイルを削除出来ます。

```sh
$ make clean
rm -rf futsush main.o prompt.o parser.o lexer.o
```


# ユニットテスト
