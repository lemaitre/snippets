#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Buffer length should be > size+1
static inline char* seek_after(FILE* fp, char delim, char* buffer, int size) {
  char* end = buffer + size;
  char* first = memchr(buffer, delim, size);
  int n = 0, i = 0;

  while (!first) {
    n = fread(buffer, 1, size, fp);
    buffer[n] = 0;
    if (n == 0) return NULL;
    first = memchr(buffer, delim, size);
  }


  i = (first - buffer) + 1;


  memmove(buffer, buffer + i, size - i);
  n = fread(end - i, 1, i, fp);
  end[n-i] = 0;

  return buffer;
}

double get_nominal_frequency() {
  char buffer[128+1];
  FILE* fp;
  const char pattern[] = "model name";
  char* line;
  char* end = &buffer[128];
  int n = 0;

  // safety
  *end = 0;

  fp = fopen("/proc/cpuinfo", "r");
  if (!fp) {
    fprintf(stderr, "Could not open /proc/cpuinfo\n");
    return 1e9;
  }

  n = fread(buffer, 1, 128, fp);
  buffer[n] = 0;
  line = n ? buffer : NULL;
  while (line) {
    // If we found the good line
    if (strncmp(pattern, line, sizeof(pattern)-1) == 0) {
      line = seek_after(fp, '@', buffer, 128);

      if (!line) break;

      char* after;
      double f = strtod(buffer, &after);

      if (after && after != line && (after = memchr(after, 'H', end - after)) && after[1] == 'z') {
        switch (after[-1]) {
          case 'G':
            f *= 1e3;
          case 'M':
            f *= 1e3;
          case 'k':
          case 'K':
            f *= 1e3;
        }
        return f;
      }
    }

    // Retrieve next line
    line = seek_after(fp, '\n', buffer, 128);
  }

  fclose(fp);
  fprintf(stderr, "Could not find nominal frequency\n");
  return 1e9;
}

int main() {
  printf("%lf Hz\n", get_nominal_frequency());
}
