#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#define isbinary(x) (iscntrl(x) || (x) == 127)

#define FILL_CHARACTER '.'

void print_usage(const char *progname) {
    printf("Usage: %s < [SAMPLE_TEXT]\n", progname);
    printf("Options:\n");
    printf("  -h, --help            Show this help message\n");
}

int main(int argc, char *argv[])
{
  unsigned long int count[0x100];
  int i, c;
  int opt;
  int option_index = 0;
  static struct option long_options[] =
    {
    /* These options set a flag. */
    {"help",     no_argument,       0, 'h'}
    };

  opt = getopt_long (argc, argv, "h:",
                long_options, &option_index);

  switch (opt) {
      case -1:
          break;
      case 'h':
          print_usage(argv[0]);
          return 0;
      default:
          print_usage(argv[0]);
          return 1;
  }

  for (i = 0; i < 0x100; i++)
    count[i]=0;

  while ((c = getchar()) != EOF)
    count[c]++;

  for (i = 0; i < 0x100; i++) {
    if (count[i]) {
      printf("0x%02x ", i);
      printf("%c %lu\n", isbinary(i) ? '.' : i, count[i]);
    }
  }

  return 0;
}
