# Linuxの3大概念
Linux(Linuxカーネル)は3つの概念で成り立っています。

* ファイル
* プロセス
* ストリーム

シンプルですが奥深い構造になっています。
たったこれだけでHDD(SSD)にファイルを保存する、端末を取り扱う、画面に表示するなどを扱うことが出来る。というよりはこれらを統一的に扱えるように抽象化した結果が上記の3点であると言えるでしょう。
それぞれの役割と関係性を簡単に説明していきましょう。

## ファイル
ファイルと言うとテキストファイルが思いつく人が大半でしょう。Linuxの場合のファイルはより広い意味で使われます。全てでは無いですが以下の種類があります。

* レギュラーファイル  
    テキストファイル, バイナリファイルなど普段扱うファイルです
* デバイスファイル  
    /dev/null, /dev/randomなどのデバイスそのものを表す特殊なファイルです。例えばUSBメモリやUSB経由ハードウェアなどを表すデバイスファイルがここに表示され、ファイルとして操作します。
* ディレクトリ  
    他のファイルを内部入れることが出来るファイルです。フォルダとも言われます。
* シンボリックリンク  
    他のファイルの名前を格納したファイルであり、(語弊ありますが)ショートカットとも呼ばれます。

## プロセス
プロセスとは動作中のプログラムのことで、動的なものでありRAM上に存在します。

一方でプログラムとはプログラムファイルであり静的なもの、大抵はSSD上に存在するファイルです。

同じプログラムから複数のプロセスを生み出すことが出来るので、それぞれを区別しなければなりません。そのため各プロセスにカーネルから個別にIDが割り当てられます。
これをプロセスID(PID)と言います。プロセスに対する操作はPIDを使用します。

## ストリーム
ストリームはデータの流れのことであり、具体的にはファイルを読み書きするときの通り道です。
この通り道の実態はカーネルが管理しているのでユーザーは直接触れませんが、通り道の入口と出口は教えられていて、これをファイルディスクリプタと呼びます。

ユーザーはファイルディスクリプタを指定して、読み・書きの操作を行います。ファイルディスクリプタの先のファイルの種類によっては出来ない操作もあります。

SSDの中のテキストファイルは読み書き両方可能ですが、USBデバイスのゲームコントローラは基本的にコントローラーからPCへの出力(ボタン押下)しかしないのでPCから見ると読み込みせず書き込みが出来ません。振動機能付きであれば振動フラグを書き込んで振動させるなどはあるかもしれません。あくまで一例です。

ストリームはファイルとプロセスが基本ですが、プロセスとプロセスの間のデータの流れも取り扱います。これはパイプと呼ばれています。シェルにおいてパイプは非常に重要です。


## まとめ
ここまでのファイル、プロセス、ストリームの関係をまとめて図にすると以下のようになります。

![linux](/doc/img/linux.svg)

# シェルの立ち位置
シェルはカーネルを守る殻のような存在という言い方をよく見かけますが、正確では無いと思っています。なぜならシェルの役割は単なるユーザーインターフェースであり、カーネルを操作するものでもなく、カーネルを保護するものでもありません。
なんだったら操作どころかカーネルにはおんぶに抱っこ状態であり、保護どころかシステムを破壊すらします。

以下の記事がとても詳しいのでシェルの自作を始める前に流し読みをオススメします。

[Qiita - 初学者のための正しいシェルとカーネルの概念 ～ 大学も技術者認定機関も間違いだらけ](https://qiita.com/ko1nksm/items/935be63e940f00e4c228)

自作を始めたあとにちょくちょく読んでみると、確かにそうそう、と思えるようになると思います。
