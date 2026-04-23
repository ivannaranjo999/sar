#include "sar.h"

static void usage(const char *name){
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "  %s p   <archive.sar> <file1..fileN>\n", name);
  fprintf(stderr, "  %s u   <archive.sar>\n", name);
  fprintf(stderr, "  %s pz  <archive.sar.gz> <file1..fileN>\n", name);
  fprintf(stderr, "  %s uz  <archive.sar.gz>\n", name);
}

int main(int argc, char *argv[]){
  /* Local variables */
  const char *command;
  const char *archive_path;
  const char **filepaths;
  const char *tmpFile = "tmp.sar";
  int count;

  /* Code */
  if (argc < 3) {
    usage(argv[0]);
    return 1;
  }

  command      = argv[1];
  archive_path = argv[2];

  if (strcmp(command, "p") == 0){
    if (argc < 4) {
      fprintf(stderr, "error: pack requires at least one file\n");
      usage(argv[0]);
      return 1;
    }

    filepaths = (const char **)&argv[3];
    count     = argc - 3;

    return pack(archive_path, filepaths, count) == 0 ? 0 : 1;
  }

  if (strcmp(command, "pz") == 0){
    if (argc < 4) {
      fprintf(stderr, "error: pack requires at least one file\n");
      usage(argv[0]);
      return 1;
    }

    filepaths = (const char **)&argv[3];
    count     = argc - 3;

    if (pack(tmpFile, filepaths, count) != 0){
      fprintf(stderr, "error: pack failed\n");
      return 1;
    }

    if (compressArch(archive_path, tmpFile) != 0) {
      fprintf(stderr, "error: compress failed\n");
      return 1;
    }

    return remove(tmpFile) == 0 ? 0 : 1;
  }

  if (strcmp(command, "u") == 0){
    return unpack(archive_path) == 0 ? 0 : 1;
  }

  if (strcmp(command, "uz") == 0){
    if(decompressArch(tmpFile, archive_path) != 0){
      fprintf(stderr, "error: decompress failed\n");
      return 1;
    }

    if(unpack(tmpFile) != 0){
      fprintf(stderr, "error: unpack failed\n");
      return 1;
    }

    return remove(tmpFile) == 0 ? 0 : 1;
  }

  fprintf(stderr, "error: unknown command '%s'\n", command);
  usage(argv[0]);
  return 1;
}
