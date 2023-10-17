# シェルを自作
作成を通してLinuxの基礎を学ぶ教育用シェルです。
ベースイメージはbashですが以下のように機能を限定して実装を簡易的にしています。

### 制限
1. 入力は1行単位
2. 対話モードのみ
3. 複合コマンド無し
    > 複合コマンドとは、( ), { }, for, if, selectなどです。詳しくは[こちら](https://linuxjm.osdn.jp/html/GNU_bash/man1/bash.1.html)。
4. 組み込みコマンドは一部のみ
    > exit, cd, pwd, jobs

### 実装内容
1. パイプライン, リダイレクト
2. ジョブ制御
    * フォアグラウンド
    * バックグラウンド
3. 端末制御
    * 行編集
    * タブ補完
    * 履歴


次のような操作はOK
```sh
command                 # フォアグラウンド実行: OK
command &               # バックグラウンド実行: OK
command_1 | command_2   # パイプライン: OK
command < file.txt      # 入力リダイレクト:OK
command > file.txt      # 出力リダイレクト新規: OK
command >> file.txt     # 出力リダイレクト追加: OK
$ xar[tab]
$ xargs                 # タブ補完: OK
$ [↑]          
$ ls -la                # 履歴: OK 
$ cd ..                 # 組み込みコマンドcd: OK
$ exit                  # 組み込みコマンドexit: OK
```

次のような操作はNG(エラーとして扱う)

```sh
$ for i in {1..5}; do echo $i;done    # for文: NG
$ `ls -la`                            # クオート展開: NG
$ ls *.c                              # ワイルドカード: NG
```

### コンセプト理由
制限している機能の多くは字句解析,構文解析を強化すれば実装が容易になるものです。

しかしシェルの最も大事な点は、ユーザー入力を端末表示でどのように補助して、コマンドを実行して終了するまでどのように管理しているのか、という点ですので構文解析や字句解析はそこまで関係ありません。

この点を抽出して強調するために構文解析は最低限に留めています。また、実装を容易にする目的もあります。

# 目次
1. [ファイルシステム、プロセス、ストリーム](/doc/linux.md)
2. [コマンド実行](/doc/command_execution.md)
    * [exec, fork, wait](/doc/command_execution.md#コマンド実行)
    * [単一コマンドを実行するプログラム](/doc/command_execution.md#単一のコマンドを実行するプログラム)
    * [ループ実行への改造](/doc/command_execution.md#ループ実行への改造)
3. [字句解析、構文解析](/doc/parsing.md)
    * [字句解析](/doc/parsing.md#字句解析)
    * [構文解析](/doc/parsing.md#構文解析)
    * [ループ実行](/doc/parsing.md#ループ実行)
4. [分割コンパイル](/doc/separate_compile.md)
    * リンク
    * Makefile
    * ユニットテスト
3. パイプライン,リダイレクト
    * pipe, dup
    * パイプライン,リダイレクトの実装
5. ジョブ制御
    * プロセスとプロセスグループ
    * バックグラウンド実行
    * シグナル
    * SIGCHLDによるジョブ制御
6. 行編集と端末
    * 端末とシェル
    * 行編集の概要
    * 行編集,制御文字検知
7. タブ補完, 履歴
    * タブ補完
    * 履歴機能