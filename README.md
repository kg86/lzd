# LZD
This is an implementation of the LZ Double-factor factorization (LZD factorization).
Each factor of the LZD factorization of a string is a concatenation of the two longest previous factors, while each factor of the LZ78 is that of the longest previous factor and the following character.

# Compile
We use [SCons](http://www.scons.org/) to build source codes.
To compile, you just type following.

```sh
./scons
```

Note thta you may have to modify some settings in SConstruct for your environment.

# Compression
## Usage
```sh
% ./out/lzd
Usage  : ./out/lzd [options]
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

Note that vfpre, vfcount and vfclean compress input file not storing whole input in main memory, but vfpre_no_stream, vfcount_no_stream do storing.

## Example
It compresses "SConstruct", and output to "hoge.lzd" by the algorithm LZD.

```sh
% out/lzd -f SConstruct -o hoge.lzd -a lzd
```

It compresses "SConstruct", and output to "hoge.vfpre10" by the algorithm LZDVF Prefix Base with code size = 10.

```sh
% out/lzd -f SConstruct -o hoge.vfpre10 -a vfpre -l 10
```

# Decompression
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

## Example
It decompresses "hoge.lzd" compressed by LZD, and output to "fuga.lzd".

```sh
% out/lzdDecompress -f hoge.lzd -o fuga.lzd -a lzd
```

It decompresses "hoge.vfpre10" compressed by LZD VF(Prefix Base) with code size = 10, and output to "fuga.vfpre10".

```sh
% out/lzdDecompress -f hoge.vfpre10 -o fuga.vfpre10 -a vfpre
```

# References
Keisuke Goto, Hideo Bannai, Shunsuke Inenaga and Masayuki Takeda. LZD Factorization: Simple and Practical Online Grammar Compression with Variable-to-Fixed Encoding, In Proceedings of the 26th Annual Symposium on Combinatorial Pattern Matching, to appear

