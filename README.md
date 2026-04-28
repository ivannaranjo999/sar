# SAR - Simple ARchiver
PENDING TEXT

```
Usage:
Actions:
  sar p   <archive.sar> <file1..fileN>     Pack given files or folders to a SAR archive.
  sar pz  <archive.sar.gz> <file1..fileN>  Pack given files or folders to a SAR archive and compress it.
  sar u   <archive.sar>                    Unpack SAR archive.
  sar uz  <archive.sar.gz>                 Unpack compressed SAR archive.
  sar l   <archive.sar>                    List files contained in a SAR archive.
  sar lz  <archive.sar.gz>                 List files contained in a compressed SAR archive.
  sar g   <archive.sar> <file1..fileN>     Grab files contained in a SAR archive.
  sar gz  <archive.sar.gz> <file1..fileN>  Grab files contained in a compressed SAR archive.
Flags:
  -v verbose output

```

## The format
SAR archives are just a flat binary file which is built as a concatenation of blocks, one per line. Each block contains a header and the file contents. The header is a fixed-size C struct storing everything needed to reconstruct the file.

```
[ FileHeader | raw bytes ][ FileHeader | raw bytes ] ...
```

## Compression
When invoked with `pz`/`uz`, SAR compresses and decompresses the entire archive using **zlib's deflate/inflate** algorithm. The result is a **gzip envelope** with the same format produced by standard `gzip` tool.

Compression is applied to the **whole archive** after packing, not per file. This ensures better compression that compressing each file individually.

## Building & Installing
List of dependencies:
- zlib

To build binary, run:
```
make
```

To install binary, run:
```
sudo make install
```

To install binary in a custom path, run:
```
make install PREFIX=<your/path>
```

To uninstall binary, run:
```
sudo make uninstall
```

To uninstall binary from a custom path, run:
```
make uninstall PREFIX=<your/path>
```
