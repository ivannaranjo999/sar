#ifndef SAR_H
#define SAR_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <utime.h>
#include <dirent.h>
#include <zlib.h>

#define SAR_MAGIC "SAR" /* Magic string at start of every header */
#define SAR_VERSION 1 /* format version */
#define SAR_MAX_PATH 4096 /* max length of stored path */
#define SAR_ARCHIVE_BUF_SIZE 1024*1024 /* 1MB read buffer */

#define COPY_BUFFER_SIZE 4096 
#define ZCHUNK 16384

typedef struct {
  char     magic[3];
  uint8_t  version;
  char     filename[SAR_MAX_PATH];
  uint64_t file_size;
  uint32_t mode;
  int64_t  mtime;
  uint8_t  _pad[4]; /* Keeps sizeof(FileHeader) a multiple of 8 */

} FileHeader;

int pack(const char *archive_path, const char **filepaths, int count, int verbose);
int unpack(const char *archive_path, int verbose);
int compressArch(const char *dst_path, const char *src_path, int verbose);
int decompressArch(const char *dst_path, const char *src_path, int verbose);
int list(const char *archive_path);

#endif
