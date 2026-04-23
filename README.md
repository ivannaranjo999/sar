# SAR - Simple ARchiver
sar is a command-line tool with two modes

sar pack <archive.sar> <list of files>
sar unpack <archive.sar>

## The format
SAR archives are just a flat binary file which is built as a concatenation of blocks, one per line.

Each block contains a header and the file contents. The header is a fixed size C struct storing everything needed to reconstruct the file.


