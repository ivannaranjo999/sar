#include "sar.h"

/* ----------------------------------------------------------------------------
 * insert
 *
 * Append to an archive at 'archive_path' all files in 
 * 'filepaths[0..count-1]'.
 * Returns 0 on success, -1 on error.
 * ------------------------------------------------------------------------- */
int insert(const char *archive_path, const char **filepaths, int count, int verbose){
  /* Local variables */
  FILE *archive;
  int   result = 0;
  int   it = 0;

  /* Code */
  archive = fopen(archive_path, "ab");
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

