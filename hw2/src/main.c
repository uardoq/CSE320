#include "debug.h"
#include "utf.h"
#include "wrappers.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

int
main(int argc, char *argv[])
{
  // test endianess
  // int x = 1;
  // char *y = (char*)&x;
  // if (*y + 48 == '1'){
  //   debug("%s","vm little endian");
  // } else {
  //   debug("%s","vm big endian");
  // }
  struct stat *file_stat;
  int infile, outfile, in_flags, out_flags;
  FILE* fp;
  int infile_nodenum, outfile_nodenum;
  parse_args(argc, argv);
  check_bom();

  // check if outputfile was given
  if (program_state->out_file == NULL) {
    // if no output file given, create one
    if ((fp = fopen("output_file.txt", "w")) == NULL) {
      // could not create file
      if (program_state != NULL) free(program_state);
      return EXIT_FAILURE;
    }
    // close file
    fclose(fp);
    // use new file
    program_state->out_file = "./output_file.txt";
  }

  print_state();

  in_flags  = O_RDONLY;
  out_flags = O_WRONLY | O_CREAT;
  infile    = Open(program_state->in_file, in_flags);
  outfile   = Open(program_state->out_file, out_flags);

  // check if infile and outfile have the same file reference
  file_stat = (struct stat*)Malloc(sizeof (struct stat));
  if (fstat(infile, file_stat) < 0) {
    free(file_stat);
    goto same_frefs;
  }
  infile_nodenum = file_stat->st_ino;
  if (fstat(outfile, file_stat) < 0) {
    free(file_stat);
    goto same_frefs;
  }
  outfile_nodenum = file_stat->st_ino;
  free(file_stat);
  if (infile_nodenum == outfile_nodenum) {
    error("same file references used, exiting...");
same_frefs:
    if (program_state != NULL) {
      free(program_state);
      close(infile);
      close(outfile);
    }
    return EXIT_FAILURE;
  }

  /* discard bom */
  if (lseek(infile, program_state->bom_length, SEEK_SET) == -1) {
    error("Could not discard BOM");
    goto same_frefs;
  }

  // Compare inode for infile and outfile
  get_encoding_function(infile, outfile)(infile, outfile);

  if (program_state != NULL) {
    free(program_state);
    close(infile);
  }

  return EXIT_SUCCESS;
}
