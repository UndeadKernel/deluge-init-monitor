#ifndef _OPTIONS_H_
#define _OPTIONS_H_

struct SOptions {
  char *config; // Configuration file to read.
  int daemonize;
};

typedef struct SOptions SOptions;

void readOptions(int argc, char *argv[], SOptions *options);
// Verify if the supplied options struct is valid.
int validOptions(SOptions *options); 
void freeOptions(SOptions *options);


#endif
