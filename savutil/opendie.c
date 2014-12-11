/*
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <savant.h>

FILE *open_or_die(char *base, char *filename, char *mode)
{
  char pathname[256];
  char errorstring[256];
  FILE *file;

  strncpy(pathname, base, 250);
  if (pathname[strlen(pathname)-1] != '/')
    strcat(pathname,"/");
  strncat(pathname, filename, 255-strlen(pathname));
  
  if((file = fopen(pathname, mode)) == NULL) {
    /*
    fprintf(stderr, "Unable to open %s\n", pathname);
    exit(-1);
    */
    sprintf(errorstring, "Unable to open filename %s", pathname);
    SavantError(ENOENT, errorstring);
  }
  else 
    return(file);
}
