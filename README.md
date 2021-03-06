# mysh

### ファイル内容

- `mysh.c` : ソースコード
- `.path` : パスファイル
- `README.md` : このファイル

### コンパイル方法

```bash
gcc -o mysh mysh.c
```

### 使用法
```bash
./mysh
```

### 説明

CUI用のシェルプログラム

起動すると以下のようなプロンプトが表示されて入力待ち状態になる

```bash
myprompt:[現在のワーキングディレクトリ]>
```

実行したいプログラム、及びプログラムに渡すオプションを入力するとプログラムを実行する

実行が終了すると再びプロンプトが表示され、入力待ち状態になる
  
また、 `ctrl + c` を入力すると実行中のプロセスを止めることができる

入力待ち状態で　`cd [ディレクトリ]` と入力すると、ワーキングディレクトリを変更することができ、 `exit` と入力すると mysh が終了する

また、複数のコマンドを `|` で繋ぐことにより、パイプ処理をすることができる

解凍時にディレクトリに含まれている `.path` は `mysh` のパス設定ファイルであり、
このファイル内にエディタでパスを通したいディレクトリを絶対パスで指定すると、`mysh`はプログラムの実行時にそのディレクトリを探索するようになる

複数のディレクトリにパスを通したい場合は、 `:` を付けてパス同士を繋げられる

例えば、/binと/sbinにパスを通したい場合以下のように記述すればよい
```
/bin:/sbin
```

#### 使用例
1. 現在のディレクトリに含まれるファイルのうち、名前に`.txt`を含むファイルを`less`で一覧表示したい場合

```bash
ls -al | grep .txt | less
```

- 加えて、 ">" と "<" を使うことで、リダイレクション処理をすることができる

2. `mysh.c`のソースコード内で `"include"` を含む行を抽出したものをgrep.txtに保存
```bash
grep include < mysh.c > grep.txt
```
もしくは
```bash
grep include > grep.txt < mysh.c
```

3. `mysh.c`のソースコード内で `"include"` を含む行を抽出し、ソートしたものを`sort.txt`に保存
```bash
grep include < mysh.c | sort > sort.txt
```

- リダイレクションとパイプによる入出力の競合が発生した場合、以下のような優先順位でプログラムが動作する

1. パイプによるプロセスの接続の前に出力先をリダイレクションで変更した場合、リダイレクションが優先される
```bash
grep include mysh.c > grep.txt | sort
```
とした場合、`grep include mysh.c` の結果が`grep.txt`に保存されて終了する

2. パイプによるプロセスの接続の後に入力元をリダイレクションで変更した場合、パイプが優先される
- `ls -la | grep include < mysh.c` とした場合、 `mysh.c` は `grep` の入力に渡されず、 `ls -la` の結果に対して `grep include` が実行される

