#include "sar.h"

static void usage(const char *name){
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "Actions:\n");
  fprintf(stderr, "  %s p   <archive.sar> <file1..fileN>\n", name);
  fprintf(stderr, "  %s u   <archive.sar>\n", name);
  fprintf(stderr, "  %s pz  <archive.sar.gz> <file1..fileN>\n", name);
  fprintf(stderr, "  %s uz  <archive.sar.gz>\n", name);
  fprintf(stderr, "Flags:\n");
  fprintf(stderr, "  -v verbose output\n");
}

int main(int argc, char *argv[]){
  /* Local variables */
  const char *action = NULL;
  const char *archive_path = NULL;
  const char **filepaths = NULL;
  const char *tmpFile = "tmp.sar";
  int i = 0;
  int verbose = 0;
  int nfiles = 0;

  /* Code */
  if (argc < 3) {
    usage(argv[0]);
    return 1;
  }

  /* Consume flags */
  for (i = 1; i < argc; ++i){
    if(strcmp(argv[i], "-v") == 0){
      verbose = 1;
      argv[i] = NULL;
    }
  }

  /* Collect positional args */
  for(i = 1; i < argc; ++i){
    if(argv[i] == NULL) continue; /* Consumed flag */
    if(action == NULL) { action = argv[i]; continue; }
    if(archive_path == NULL) { archive_path = argv[i]; continue; }
    /* From here just files */
    if(filepaths  == NULL) { filepaths = (const char **)&argv[i]; }
    nfiles++;
  }

  if (action == NULL || archive_path == NULL){
    usage(argv[0]);
    return 1;
  }

  if (strcmp(action, "p") == 0){
    if (nfiles == 0) {
      fprintf(stderr, "error: 'p' requires at least one file\n");
      usage(argv[0]);
      return 1;
    }

    return pack(archive_path, filepaths, nfiles, verbose) == 0 ? 0 : 1;

  } else if (strcmp(action, "pz") == 0){
    if (nfiles == 0) {
      fprintf(stderr, "error: 'pz' requires at least one file\n");
      usage(argv[0]);
      return 1;
    }

    if (pack(tmpFile, filepaths, nfiles, verbose) != 0){
      fprintf(stderr, "error: pack failed\n");
      return 1;
    }

    if (compressArch(archive_path, tmpFile, verbose) != 0) {
      fprintf(stderr, "error: compress failed\n");
      return 1;
    }

    return remove(tmpFile) == 0 ? 0 : 1;

  } else if (strcmp(action, "u") == 0){
    return unpack(archive_path, verbose) == 0 ? 0 : 1;

  } else if (strcmp(action, "uz") == 0){
    if(decompressArch(tmpFile, archive_path, verbose) != 0){
      fprintf(stderr, "error: decompress failed\n");
      return 1;
    }

    if(unpack(tmpFile, verbose) != 0){
      fprintf(stderr, "error: unpack failed\n");
      return 1;
    }

    return remove(tmpFile) == 0 ? 0 : 1;

  } else {
    fprintf(stderr, "error: unknown action '%s'\n", action);
    usage(argv[0]);
    return 1;
  }
}
