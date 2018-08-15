#include "debug.h"
#include "utf.h"
// #include "wrappers.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int opterr;
int optopt;
int optind;
char *optarg;

state_t *program_state;

void
parse_args(int argc, char *argv[])
{
  int i = 0;
  char option;
  char *joined_argv;

  if (argc < 2 || argc > 5){
    error("wrong num of args: %d", argc);
    USAGE(argv[0]);
    exit(EXIT_FAILURE);
  }

  // join_string_array concatenates each arg into 1 big string?
  joined_argv = join_string_array(argc, argv);
  info("argc: %d argv: %s", argc, joined_argv);
  free(joined_argv);

  // allocate 1 block size of sizeof(state_t) = 32 bytes
  program_state = Calloc(1, sizeof(state_t));
  while ((option = getopt(argc, argv, "e:h")) != -1) {
    debug("%d opterr: %d", i, opterr);
    debug("%d optind: %d", i, optind);
    debug("%d optopt: %d", i, optopt);
    debug("%d argv[optind]: %s", i, argv[optind]);
    switch (option) {
    case 'h': {
showhelp:
      // show help menu
      USAGE(argv[0]);
      free(program_state);
      exit(EXIT_SUCCESS);
      break;
    }
    case 'e': {
      // case if optarg is -h, treat as -h flag
      if (strcmp(optarg, "-h") == 0) {
        goto showhelp;
      }
      // optarg can only contain UTF16LE, UTF16BE, UTF8
      info("Encoding Argument: %s", optarg);
      // check encoding arg
      if ((program_state->encoding_to = determine_format(optarg)) == 0) {
        // also handles case when the -e arg is missing but infile or outfile was given
        error( "encoding to %s is not supported\n", optarg);
        // bad encoding arg
        goto errorcase;
      }
      break;
    }
    case '?': {
      if (optopt == 'e') {
        error( "option -%c needs an argument\n", optopt);
      }
      else {
        error( "-%c is not a supported argument\n", optopt);
      }
      goto errorcase;
      break;
    }
    default: {
errorcase:
      free(program_state);
      exit(EXIT_FAILURE);
    }
    }
    i++;
  }
  while ( optind < argc ) {
    if (argv[optind + i - 1] != NULL)
    {
      if (program_state->in_file == NULL) {
        program_state->in_file = argv[optind + i - 1];
      }
      else if (program_state->out_file == NULL)
      {
        program_state->out_file = argv[optind + i - 1];
      }
      optind++;
    }
  }
}

format_t
determine_format(char *argument)
{
  if (strcmp(argument, STR_UTF16LE) == 0)
    return UTF16LE;
  if (strcmp(argument, STR_UTF16BE) == 0)
    return UTF16BE;
  if (strcmp(argument, STR_UTF8) == 0)
    return UTF8;
  return 0;
}

const char*
bom_to_string(format_t bom) {
  switch (bom) {
  case UTF8: return STR_UTF8;
  case UTF16BE: return STR_UTF16BE;
  case UTF16LE: return STR_UTF16LE;
  }
  return "UNKNOWN";
}

char*
join_string_array(int count, char *array[])
{
  char *ret;
  char charArray[count];
  int i;
  int len = 0, str_len, cur_str_len;

  // point ret to charArray[0]
  ret = charArray;
  // allocate space for array
  str_len = array_size(count, array);
  ret = calloc(str_len, sizeof(char));

  for (i = 0; i < count; ++i) {
    // get num of chars in string
    cur_str_len = strlen(array[i]);
    // ret + len = last char copied location + 1
    // cur_str_len says how many chars to copy from array[i] to charArray
    memecpy(ret + len, array[i], cur_str_len);
    len += cur_str_len;
    // adds a space between the strings in array
    memecpy(ret + len, " ", 1);
    len += 1;
  }
  // add null terminator
  *(ret + len) = 0;
  return ret; // free me after use
}

/**
 * returns the number of chars in an array of strings
 */
int
array_size(int count, char *array[])
{
  int i, sum = 1; /* NULL terminator */
  for (i = 0; i < count; ++i) {
    sum += strlen(array[i]);
    ++sum; /* For the spaces */
  }
  return sum;
}

void
print_state()
{
  if (program_state == NULL) {
    error("program_state is %p", (void*)program_state);
    exit(EXIT_FAILURE);
  }
  info("program_state {\n"
       "  format_t encoding_to = 0x%X;\n"
       "  format_t encoding_from = 0x%X;\n"
       "  char *in_file = '%s';\n"
       "  char *out_file = '%s';\n"
       "};\n",
       program_state->encoding_to, program_state->encoding_from,
       program_state->in_file, program_state->out_file);
}
