#include "stools.h"
#include <string.h>
#include <stdlib.h>


char *copyString(const char *str)
{
  int str_len = strlen(str);
  char *str_copy;

  // Make enough space for the copy.
  str_copy = malloc(str_len + 1);

  // Copy including the null terminating byte.
  memcpy(str_copy, str, str_len + 1);

  return str_copy;
}
