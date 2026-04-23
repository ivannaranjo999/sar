#include "sar.h"
/* ----------------------------------------------------------------------------
 * mkdir_parents
 *
 * Creates all parent directories for a given filepath.
 * Returns 0 on success, -1 on error.
 * ------------------------------------------------------------------------- */
static int mkdir_parents(const char *filepath){
  char  tmp[SAR_MAX_PATH];
  char *p;

  strncpy(tmp, filepath, SAR_MAX_PATH - 1);
  tmp[SAR_MAX_PATH - 1] = '\0';

  /* Walk every '/' in the path and mkdir up to that point */
  for (p = tmp + 1; *p; p++){
    if (*p == '/'){
      *p = '\0';
      if (mkdir(tmp, 0755) != 0 && errno != EEXIST){
        perror(tmp);
        return -1;
      }
      *p = '/';
    }
  }
  return 0;
}

/* ----------------------------------------------------------------------------
 * unpack_file
 *
 * Read one (header + data) block from the archive and write the file to disk.
 * The archive must be positioned at the start of a FileHeader when this is
 * called.
 * Returns 1 if a file was extracted successfully,
 *         0 if we have reached EOF
 *        -1 on error
 * ------------------------------------------------------------------------- */
static int unpack_file(FILE *archive){
  /* Local variables */
  FileHeader     header;
  FILE          *dst;
  char           buf[COPY_BUFFER_SIZE];
  uint64_t       remaining;
  size_t         bytes_read, to_write, n, chunk;
  struct utimbuf times;

  /* Code */
  /* Read header */
  n = fread(&header, sizeof(header), 1, archive);

  if(n == 0) {
    if(feof(archive)) return 0;
    perror("fread header");
    return -1;
  }

  /* Validate magic bytes */
  if(memcmp(header.magic, SAR_MAGIC, 3) != 0){
    fprintf(stderr, "error: bad magic - make sure this is a sar archive\n");
    return -1;
  }

  /* Validate version */
  if(header.version != SAR_VERSION){
    fprintf(stderr, "error: unsupported archive version %d\n", header.version);
    return -1;
  }

  /* Ensure filename is null terminated */
  header.filename[SAR_MAX_PATH - 1] = '\0';

  /* Create parent dirs if needed */
  if(mkdir_parents(header.filename) != 0){
    fseek(archive, (long)header.file_size, SEEK_CUR);
    fprintf(stderr, "error: could not create parent dirs for '%s'\n", header.filename);
    return -1;
  }

  /* Open destination file */
  dst = fopen(header.filename, "wb");
  if (dst == NULL){
    perror(header.filename);
    fseek(archive, (long)header.file_size, SEEK_CUR);
    return -1;
  }

  /* Copy data from archive to destination file */
  remaining = header.file_size;

  while(remaining > 0){
    chunk = remaining < sizeof(buf) ? remaining : sizeof(buf);

    bytes_read = fread(buf, 1, chunk, archive);
    if(bytes_read == 0){
      fprintf(stderr,
        "error: unexpected end of archive while reading '%s'\n",
        header.filename);
      fclose(dst);
      return -1;
    }
    
    to_write = bytes_read;
    if(fwrite(buf, 1, to_write, dst) != to_write){
      fprintf(stderr,
        "error: failed to write to '%s'\n",
        header.filename);
      fclose(dst);
      return -1;
    }

    remaining -= bytes_read;
  }

  fclose(dst);

  /* Restore permissions */
  if(chmod(header.filename, (mode_t)header.mode) != 0){
    perror("chmod");
  }

  /* Restore modification time */
  times.actime  = (time_t)header.mtime; /* access time = mtime */
  times.modtime = (time_t)header.mtime; /* last modified time */

  if (utime(header.filename, &times) != 0) {
    perror("utime");
  }

  printf("unpacked: '%s' (%llu bytes)\n",
    header.filename, (unsigned long long)header.file_size);

  return 1;
}

/* ----------------------------------------------------------------------------
 * unpack
 *
 * Extract all files from the archive at 'archive_path'.
 * Returns 0 on success, -1 if any file failed.
 * ------------------------------------------------------------------------- */
int unpack(const char *archive_path){
  /* Local variables */
  FILE *archive;
  int   result = 0;
  int   status;

  /* Code */
  archive = fopen(archive_path, "rb");
  if(archive == NULL){
    perror(archive_path);
    return -1;
  }

  while((status = unpack_file(archive)) == 1){
    /* Keep going until EOF or error */
  }

  if(status == -1) result = -1;

  fclose(archive);
  return result;
}

/* ----------------------------------------------------------------------------
 * decompressArch
 *
 * Decompress to 'dst' from 'src'
 * Returns 0 on success, -1 on error.
 * ------------------------------------------------------------------------- */
int decompressArch(const char *dst_path, const char *src_path){
  /* Local variables */
  int ret;
  unsigned have;
  z_stream strm;
  unsigned char in[ZCHUNK];
  unsigned char out[ZCHUNK];
  FILE *dst;
  FILE *src;

  /* Code */
  dst = fopen(dst_path, "wb");
  if(dst == NULL){
    fprintf(stderr, "error: could not open '%s'\n", dst_path);
    return -1;
  }

  src = fopen(src_path, "rb");
  if(src == NULL){
    fprintf(stderr, "error: could not open '%s'\n", dst_path);
    fclose(dst);
    return -1;
  }

  /* Initialize the zlib stream for decompression */
  strm.zalloc   = Z_NULL;
  strm.zfree    = Z_NULL;
  strm.opaque   = Z_NULL;
  strm.avail_in = 0;
  strm.next_in  = Z_NULL;
  ret = inflateInit2(&strm, 15 + 16);
  if (ret != Z_OK) {
    fclose(dst);
    fclose(src);
    return -1;
  }

  /* Decompress until EOF */
  do{
    strm.avail_in = fread(in, 1, ZCHUNK, src);
    if(ferror(src)){
      (void)inflateEnd(&strm);
      fclose(dst);
      fclose(src);
      return -1;
    }
    if(strm.avail_in == 0) break;
    strm.next_in = in;

    do{
      strm.avail_out = ZCHUNK;
      strm.next_out  = out;
      ret = inflate(&strm, Z_NO_FLUSH);
      if (ret == Z_STREAM_ERROR  ||
          ret == Z_NEED_DICT     ||
          ret == Z_DATA_ERROR    ||
          ret == Z_MEM_ERROR) {
        fprintf(stderr, "error: failure during decompression (%d)\n", ret);
        (void)inflateEnd(&strm);
        fclose(dst);
        fclose(src);
        return -1;
      }
      have = ZCHUNK - strm.avail_out;
      if (fwrite(out, 1, have, dst) != have || ferror(dst)) {
        (void)inflateEnd(&strm);
        fclose(dst);
        fclose(src);
        return -1;
      }
    } while (strm.avail_out == 0);
  } while (ret != Z_STREAM_END);

  if (ret != Z_STREAM_END) {
    fprintf(stderr, "error: incomplete or corrupt gzip stream\n");
    (void)inflateEnd(&strm);
    fclose(dst);
    fclose(src);
    return -1;
  }

  /* Clean up */
  (void)inflateEnd(&strm);
  fclose(dst);
  fclose(src);
  return 0;
}
