#include "sar.h"

/* ----------------------------------------------------------------------------
 * pack_file
 *
 * Write one file into the open archive.
 * Returns 0 on success, -1 on error.
 * ------------------------------------------------------------------------- */
static int pack_file(FILE *archive, const char *filepath, int verbose){
  /* Local variables */
  struct stat    st;
  FILE          *src;
  DIR           *dir;
  struct dirent *entry;
  FileHeader     header;
  char           buf[COPY_BUFFER_SIZE];
  char           fullpath[SAR_MAX_PATH];
  size_t         bytes_read;
  int            result = 0;

  /* Code */
  /* Read metadata */
  if (stat(filepath, &st) != 0) {
    perror(filepath);
    return -1;
  }

  /* If dir, recurse */
  if(S_ISDIR(st.st_mode)){
    dir = opendir(filepath);
    if (dir == NULL){
      perror(filepath);
      return -1;
    }
    while((entry = readdir(dir)) != NULL){
      if(strcmp(entry->d_name, ".") == 0) continue;
      if(strcmp(entry->d_name, "..") == 0) continue;
      if (snprintf(fullpath, sizeof(fullpath), "%s/%s",
          filepath, entry->d_name) >= (int)sizeof(fullpath)){
        fprintf(stderr, "error: path too long: '%s/%s'\n", 
          filepath, entry->d_name);
        result = -1;
        continue;
      }
      if (pack_file(archive, fullpath, verbose) != 0) result = -1;
    }
    closedir(dir);
    return result;
  }

  /* Operate only regular files */
  if (!S_ISREG(st.st_mode)) {
    fprintf(stderr, "skipping '%s': not a regular file\n", filepath);
    return -1;
  }

  /* Open file */
  src = fopen(filepath, "rb");
  if (src == NULL){
    perror(filepath); 
    return -1;
  }

  /* Fill header info */
  memset(&header, 0, sizeof(header));
  memcpy(header.magic, SAR_MAGIC, 3);
  header.version   = SAR_VERSION;
  header.file_size = (uint64_t)st.st_size;
  header.mode      = (uint32_t)st.st_mode;
  header.mtime     = (int64_t)st.st_mtime;
  strncpy(header.filename, filepath, SAR_MAX_PATH - 1);
  header.filename[SAR_MAX_PATH - 1] = '\0';

  /* Write header to archive */
  if (fwrite(&header, sizeof(header), 1, archive) != 1){
    fprintf(stderr, "error: failed to write header for '%s'\n", filepath);
    fclose(src);
    return -1;
  }

  /* Copy file data into archive */
  while ((bytes_read = fread(buf, 1, sizeof(buf), src)) > 0){
    if(fwrite(buf, 1, bytes_read, archive) != bytes_read){
      fprintf(stderr, "error: failed to write data for '%s'\n", filepath);
      fclose(src);
      return -1;
    }
  }

  if (ferror(src)){
    fprintf(stderr, "error: failed to read from '%s'\n", filepath);
    fclose(src);
    return -1;
  }

  fclose(src);
  if (verbose)
    printf("packed: '%s' (%llu + %llu bytes)\n", 
      filepath, (unsigned long long)sizeof(FileHeader),
      (unsigned long long)st.st_size);

  return 0;
}

/* ----------------------------------------------------------------------------
 * pack
 *
 * Create an archive at 'archive_path' containing all files in 
 * 'filepaths[0..count-1]'.
 * Returns 0 on success, -1 on error.
 * ------------------------------------------------------------------------- */
int pack(const char *archive_path, const char **filepaths, int count, int verbose){
  /* Local variables */
  FILE *archive;
  int   result = 0;
  int   it = 0;

  /* Code */
  archive = fopen(archive_path, "wb");
  if (archive == NULL){
    perror(archive_path);
    return -1;
  }
  setvbuf(archive, NULL, _IOFBF, SAR_ARCHIVE_BUF_SIZE);

  for (it = 0; it < count; ++it){
    if(pack_file(archive, filepaths[it], verbose) != 0){
      result = -1; /* record failure but keep packing */
    }
  }

  fclose(archive);
  return result;
}

/* ----------------------------------------------------------------------------
 * compressArch
 *
 * Compress to 'dst' from 'src'
 * Returns 0 on success, -1 on error.
 * ------------------------------------------------------------------------- */
int compressArch(const char *dst_path, const char *src_path, int verbose){
  /* Local variables */
  int ret, flush;
  unsigned have;
  z_stream strm;
  unsigned char in[ZCHUNK];
  unsigned char out[ZCHUNK];
  FILE *dst;
  FILE *src;

  /* Code */
  if (verbose)
    printf("compressing to '%s'\n", 
      dst_path);

  dst = fopen(dst_path, "wb");
  if (dst == NULL) {
    fprintf(stderr, "error: could not open '%s'\n", dst_path);
    return -1;
  }
  setvbuf(dst, NULL, _IOFBF, SAR_ARCHIVE_BUF_SIZE);

  src = fopen(src_path, "rb");
  if (src == NULL) {
    fprintf(stderr, "error: could not open '%s'\n", src_path);
    fclose(dst);
    return -1;
  }
  setvbuf(src, NULL, _IOFBF, SAR_ARCHIVE_BUF_SIZE);

  /* Initialize the zlib stream for compression */
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  ret = deflateInit2(&strm,
                     Z_DEFAULT_COMPRESSION,
                     Z_DEFLATED,
                     15+16,
                     8,
                     Z_DEFAULT_STRATEGY);
  if (ret != Z_OK){
    return -1;
    fclose(dst);
    fclose(src);
  } 

  /* Compress until EOF */
  do {
    strm.avail_in = fread(in, 1, ZCHUNK, src);
    if (ferror(src)) {
      (void)deflateEnd(&strm);
      return -1;
    }
    flush = feof(src) ? Z_FINISH : Z_NO_FLUSH;
    strm.next_in = in;

    /* Run deflate() on input until output buffer not full */
    do {
      strm.avail_out = ZCHUNK;
      strm.next_out = out;
      ret = deflate(&strm, flush);
      if(ret == Z_STREAM_ERROR){
        fprintf(stderr, "error: failure during compression\n");
        fclose(dst);
        fclose(src);
        return -1;
      }
      have = ZCHUNK - strm.avail_out;
      if (fwrite(out, 1, have, dst) != have || ferror(dst)) {
        (void)deflateEnd(&strm);
        fclose(dst);
        fclose(src);
        return -1;
      }
    } while (strm.avail_out == 0);
    if (strm.avail_in != 0){
      fprintf(stderr, "error: failure during compression\n");
      fclose(dst);
      fclose(src);
      return -1;
    }
  } while (flush != Z_FINISH);
  if(ret != Z_STREAM_END){
      fprintf(stderr, "error: failure during compression\n");
      fclose(dst);
      fclose(src);
      return -1;
  }

  /* Clean up */
  (void)deflateEnd(&strm);

  fclose(dst);
  fclose(src);

  return 0;
}
