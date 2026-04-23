# SAR - Simple ARchiver
sar is a command-line tool with two modes

```
# Pack files to create a .sar archive
sar p   <archive.sar> <file1..fileN>

# Unpack .sar to files
sar u   <archive.sar>

# Pack files to a .sar archive and compress it using zlib
sar pz  <archive.sar.gz> <file1..fileN>

# Unpack compressed .sar file
sar uz  <archive.sar.gz>
```

## The format
SAR archives are just a flat binary file which is built as a concatenation of blocks, one per line. Each block contains a header and the file contents. The header is a fixed-size C struct storing everything needed to reconstruct the file.

```
[ FileHeader | raw bytes ][ FileHeader | raw bytes ] ...
```

## Compression
When invoked with `pz`/`uz`, SAR compresses and decompresses the entire archive using **zlib's deflate/inflate** algorithm. The result is a **gzip envelope** with the same format produced by starndard `gzip` tool.

Compression is applied to the **whole archive** after packing, not per file. This ensures better compression that compressing each file individually.
