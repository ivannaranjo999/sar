#include "sar.h"

/* ----------------------------------------------------------------------------
 * filename_matches
 *
 * See if given filename matches a list of filepaths.
 * Returns 0 on success, -1 if any file failed.
 * ------------------------------------------------------------------------- */
static int filename_matches(const char *archived_name, const char **filepaths, int count){
  /* Local variables */
  int index = 0;

  /* Code */
  for (index = 0; index < count ; ++index){
    if (strstr(archived_name, filepaths[index]) != NULL) return -1;
  }
  return 0;
}

/* ----------------------------------------------------------------------------
 * grab
 *
 * Extract given files from the archive at 'archive_path'.
 * Returns 0 on success, -1 if any file failed.
 * ------------------------------------------------------------------------- */
int grab(const char *archive_path, const char **filepaths, int count, int verbose){
  /* Local variables */
  FILE      *archive;
  int        result = 0;
  int        status = 0;
  int        exitLoop = 0;
  int        matched = 0;
  size_t     n = 0;
  FileHeader header;

  /* Code */
  archive = fopen(archive_path, "rb");
  if(archive == NULL){
    perror(archive_path);
    return -1;
  }
  setvbuf(archive, NULL, _IOFBF, SAR_ARCHIVE_BUF_SIZE);

  /* Read first block */
  n = fread(&header, sizeof(header), 1, archive);

  if(n == 0) {
    if(feof(archive)){
      exitLoop = 1;
    } else {
      perror("fread header");
      result = -1;
    }
  }

  /* Exit loop when EOF reached */
  while(exitLoop == 0){
    matched = 0;

    /* Check if next block is any of the files to find */
    if(filename_matches(header.filename, filepaths, count)){
      if (verbose) fprintf(stdout, "grab: found file '%s'\n", header.filename);
      /* Jump to beginning of the block */
      fseek(archive, -sizeof(FileHeader), SEEK_CUR);

      /* Extract file */
      status = unpack_file(archive, verbose);
      if(status == -1) {
        result = -1;
        fprintf(stderr, "error: could not unpack '%s'\n", header.filename);
      }

      matched = 1;
    }

    if (matched != 1) {
      /* Jump to next file if not found */
      fseek(archive, (long)header.file_size, SEEK_CUR);
    }

    /* Read next block */
    n = fread(&header, sizeof(header), 1, archive);

    if(n == 0) {
      /* Check if EOF */
      if(feof(archive)){
        exitLoop = 1;
      } else {
        /* Error found */
        perror("fread header");
        status = -1;
      }
    }
  }

  fclose(archive);
  return result;
}
