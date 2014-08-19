#include "options.h"
#include "stools.h"

#include <getopt.h>
#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>


void readOptions(int argc, char *argv[], SOptions *options)
{
  int option_index = 0;
  int option_character = 0;

  // Do not input error messages for invalid options.
  opterr = 0;

  static struct option long_options[] = {
    {"conf", required_argument, (int*)0, 'c'},
    {"daemonize", no_argument, (int*)0, 'd'},
    {0, 0, 0, 0}
  };

  // Do we have a valid options structure?
  if(options == NULL) {
    syslog(LOG_ERR, "BUG: Invalid options structure specified.");
    exit(1);
  }

  // Set default values for some options.
  options->daemonize = 0;
  
  // Start the parsing of the optoins.
  while(1) {
    option_character = getopt_long(argc, argv, ":c:d", long_options, &option_index);

    // Are there still options to parse?
    if(option_character == -1) {
      // No more options to parse.
      break;
    }

    switch(option_character) {
    case 'c':
      // Config file supplied as an argument.
      options->config = copyString(optarg);
      syslog(LOG_INFO, "Configuration file: %s \n", options->config);
      break;

    case 'd':
      // We are to run as a daemon.
      options->daemonize = 1;
      syslog(LOG_INFO, "Running as a daemon.");
      break;

    case '?':
      // Error getting options.
      if(optopt) {
	syslog(LOG_ERR, "Unknown option: %c\n", optopt);
      }
      else {
	syslog(LOG_ERR, "Unknown option: %s\n", argv[optind - 1]);
      }
      exit(1);
      break;

    case ':':
      // Error getting an option argument.
      syslog(LOG_ERR, "An argument was not specified for: %c\n", optopt);
      exit(1);
      break;
    }
  }
}

int validOptions(SOptions *options)
{
  // Check for a valid config.
  if(options->config == NULL || strlen(options->config) < 1) {
    return 0;
  }

  // Check for a valid daemonize value.
  if(options->daemonize < 0 || options->daemonize > 1) {
    return 0;
  }

  return 1;
}

void freeOptions(SOptions *options)
{
  if(options->config != NULL) {
    free(options->config);
  }
}
