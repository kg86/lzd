# LZD
This is an implementation of the LZ Double-factor factorization (LZD factorization).
LZD is a simple extension of the well-known compression algorithm LZ78.
While LZ78 factorize an input string to the sequence of pairs of a longest previous factor and the succeeding character, 
LZD factorize an input string to the sequence of pairs of a longest previous factor and the succeeding longest previous factor.
LZD shows better compression ratio than LZ78 in practical.


# Compile
We use [SCons](http://www.scons.org/) to build source codes.
To compile, you just type following command in the top of the project directory.
Binary files are put in `out` directory.

```sh
$ scons
```

Note that you may have to modify some settings such as compiler in SConstruct for your environment.


# Compress
## Usage
```sh
$ out/lzd
Usage  : out/lzd [options]
Options: 
  -f FileName         : input file
  -o FileName         : output file
  -c                  : check whether decompressed string equals the input
  -d NUM              : set the debug level
  -l maxSize          : set max code size
  -a lz78       : LZ78
  -a lzd        : LZD
  -a vfpre      : LZD VF (Prefix Base)
  -a vfcount    : LZD VF (Count Base)
  -a vfclean    : LZD VF (Reset Base)
  -a vfpre_no_stream   : LZD VF (Prefix Base)
  -a vfcount_no_stream : LZD VF (Count Base)
```


Note that vfpre_no_stream, vfcount_no_stream store the input file in main memory while the compression, but vfpre, vfcount and vfclean do not.

## Examples
The following command compresses `SConstruct`, and output to `hoge.lzd` by the algorithm LZD.

```sh
$ out/lzd -f SConstruct -o hoge.lzd -a lzd
```

The following command compresses `SConstruct`, and output to `hoge.vfpre10` by the algorithm LZDVF Prefix Base with code size = 10.

```sh
$ out/lzd -f SConstruct -o hoge.vfpre10 -a vfpre -l 10
```

# Decompress
## Usage
```sh
% out/lzdDecompress
Usage  : out/lzdDecompress [options]
Options               : 
  -f FileName         : input file
  -o FileName         : output file
  -d NUM              : debug mode
  -a lz78       : LZ78
  -a lzd        : LZD
  -a vfpre      : LZD VF (Prefix Base)
  -a vfcount    : LZD VF (Count Base)
  -a vfclean    : LZD VF (Reset Base)
  -a vfpre_no_stream   : LZD VF (Prefix Base)
  -a vfcount_no_stream : LZD VF (Count Base)
```

## Examples
The following command decompresses `hoge.lzd` compressed by LZD, and output to `fuga.lzd`.

```sh
$ out/lzdDecompress -f hoge.lzd -o fuga.lzd -a lzd
```

The following decompresses `hoge.vfpre10` compressed by LZD VF(Prefix Base) with code size = 10, and output to `fuga.vfpre10`.

```sh
$ out/lzdDecompress -f hoge.vfpre10 -o fuga.vfpre10 -a vfpre
```

# References
The detail of the algorithm was described in the following paper.

>Keisuke Goto, Hideo Bannai, Shunsuke Inenaga and Masayuki Takeda. LZD Factorization: Simple and Practical Online Grammar Compression with Variable-to-Fixed Encoding, In Proceedings of the 26th Annual Symposium on Combinatorial Pattern Matching

# License
Copyright © 2015 Keisuke Goto. This software released under the MIT License.
