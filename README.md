# 1000行で作る自作シェル(C言語編)
作成を通してLinuxの基礎を学ぶ教育用シェルです。
ベースイメージはbashですが以下のように機能を限定して実装を簡易的にしています。

### 制限
1. 入力は1行単位のみ
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

具体例を上げると、次のような操作はOK
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

次のような操作は出来ません。(エラーとして扱う)

```sh
$ for i in {1..5}; do echo $i;done    # for文: NG
$ `ls -la`                            # クオート展開: NG
$ ls *.c                              # ワイルドカード: NG
```

### コンセプト理由
制限している機能の多くは字句解析,構文解析を強化すれば実装が容易になるものです。
しかしシェルの最も大事な点は、ユーザー入力を端末表示でどのように補助して、コマンドを実行して終了するまでどのように管理しているのか、という点ですので構文解析や字句解析はそこまで関係ありません。この点を抽出して強調するために構文解析は最低限に留めています。
また構文解析をしっかりやろうとすると途端に実装が大変になるので、実装を容易にする目的もあります。

# 目次
1. Linuxにおけるファイルとデバイス, ファイルディスクリプタ
2. コマンド実行
    * exec, wait
3. パイプライン
    * 字句解析-構文解析
    * fork, pipe, dup
4. リダイレクト
5. ジョブ制御
    * プロセスとプロセスグループ
    * シグナル
6. 行編集と端末
7. タブ補完, 履歴