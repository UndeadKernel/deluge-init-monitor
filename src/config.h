#ifndef _CONFIG_H_
#define _CONFIG_H

#include "options.h"


int openConfig(const char* file_path);
void closeConfig();
const char *configFileError();
int configLineError();
const char *configTextError();
int configReadInt(const char *key, int *value);
int configReadStr(const char *key, const char* value[]);


#endif
