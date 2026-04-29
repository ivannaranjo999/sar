#include "sar.h"

/* ----------------------------------------------------------------------------
 * get_filename
 *
 * Read one (header + data) block from the archive and write the file to disk.
 * The archive must be positioned at the start of a FileHeader when this is
 * called.
 * Returns 1 if a file was extracted successfully,
 *         0 if we have reached EOF
 *        -1 on error
 * ------------------------------------------------------------------------- */
static int get_filename(FILE *archive){
  /* Local variables */
  FileHeader header;
  size_t n;

  /* Code */
  /* Read header */
  n = fread(&header, sizeof(header), 1, archive);

  if(n == 0) {
    if(feof(archive)) return 0;
    perror("fread header");
    return -1;
  }

  /* Ensure filename is null terminated */
  header.filename[SAR_MAX_PATH - 1] = '\0';

  /* Print filename */
  fprintf(stdout, "%s\n", header.filename);

  /* Jump to next block */
  fseek(archive, (long)header.file_size, SEEK_CUR);

  return 1;
}

/* ----------------------------------------------------------------------------
 * list
 *
 * Shows all filenames inside a .sar file
 * Returns 0 on success, -1 on error.
 * ------------------------------------------------------------------------- */
int list(const char *archive_path){
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
  setvbuf(archive, NULL, _IOFBF, SAR_ARCHIVE_BUF_SIZE);

  while((status = get_filename(archive)) == 1){
    /* Keep going until EOF or error */
  }

  if(status == -1) result = -1;

  fclose(archive);
  return result;
}