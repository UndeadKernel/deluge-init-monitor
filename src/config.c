#include <libconfig.h>

#include "config.h"
#include "options.h"

static config_t config;


int openConfig(const char* file_path)
{
  int read_status = 0;

  config_init(&config);
  read_status = config_read_file(&config, file_path);

  return read_status;
}

void closeConfig()
{
  config_destroy(&config);
}

const char *configFileError()
{
  return config_error_file(&config);
}

int configLineError()
{
  return config_error_line(&config);
}

const char *configTextError()
{
  return config_error_text(&config);
}

int configReadInt(const char *key, int *value)
{
  return config_lookup_int(&config, key, value);
}

int configReadStr(const char *key, const char* value[])
{
  return config_lookup_string(&config, key, value);
}
