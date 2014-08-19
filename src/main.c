#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

#include <libconfig.h>

#include "options.h"
#include "config.h"
#include "monitor.h"
#include "stools.h"


void openSysLog(char *ident);
void daemonize();
void forkIntoBg();
void newSession();
void redirectDescriptors();
void setupSignals();
void signalHandler(int signal);
void setupMonitorParams(SMonitorParams *monitor_params);
void freeMonitorParams(SMonitorParams *monitor_params);


int main(int argc, char **argv)
{
  // Open the syslog.
  openSysLog(argv[0]);

  // Read the user defined options.
  SOptions options = {0};
  readOptions(argc, argv, &options);
  if(!validOptions(&options)) {
    syslog(LOG_ERR, "Some required options were not specified. Exiting..");
    exit(1);
  }

  // Go into daemon mode.
  if(options.daemonize == 1) {
    daemonize();
  }

  // Read the config file.
  if(!openConfig(options.config)) {
    // Reading the configuration file failed.
    syslog(LOG_ERR, "Error reading the configuration file:");
    syslog(LOG_ERR, "%s:%i  %s", 
	   configFileError(), 
	   configLineError(), 
	   configTextError());
    exit(1);
  }

  // The command line options are no longer needed.
  // ... Free the options' memory.
  freeOptions(&options);

  // Set up the parameters of the monitor.
  SMonitorParams monitor_params;
  setupMonitorParams(&monitor_params);

  // Done reading the configuration.
  closeConfig();
  
  // Start monitoring.
  startMonitor(&monitor_params);

  // Free the monitor parameters.
  freeMonitorParams(&monitor_params);

  return 0;
}


void openSysLog(char *ident)
{
  openlog(ident, LOG_NOWAIT | LOG_PID, LOG_USER);
}

void daemonize()
{
  // Fork into background.
  forkIntoBg();
  
  // Set the umask.
  umask(0);

  // Close the standard file descriptors.
  redirectDescriptors();

  // Create a new process group.
  newSession();

  // Change current directory.
  if(chdir("/") == -1) {
    syslog(LOG_ERR, "Failed to change working directory to /");
    exit(1);
  }

  setupSignals();
}

void forkIntoBg()
{
  pid_t pid;

  // Fork into the background.
  pid = fork();
  if(pid < 0) {
    // Forking failed.
    syslog(LOG_ERR, "Failed to fork in the background.");
    exit(1);
  }
  else if(pid > 0) {
    // The original program.
    // ... Our child will carry on.
    exit(0);
  }

}

void newSession()
{
  pid_t sid;

  sid = setsid();

  if(sid < 0) {
    syslog(LOG_ERR, "Failed to create a new process group.");
    exit(1);
  }
}

void redirectDescriptors()
{
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  open("/dev/null", O_RDONLY);
  open("/dev/null", O_RDWR);
  open("/dev/null", O_RDWR);
}

void setupSignals()
{
  struct sigaction saction;
  struct sigaction signore;

  saction.sa_handler = signalHandler;
  sigfillset(&saction.sa_mask);
  saction.sa_flags = 0;

  signore.sa_handler = SIG_IGN;
  sigemptyset(&signore.sa_mask);
  signore.sa_flags = 0;

  // Redirect some signals.
  sigaction(SIGTERM, &saction, NULL);
  sigaction(SIGINT, &saction, NULL);
  sigaction(SIGHUP, &saction, NULL);

  // Ignore some other signals.
  sigaction(SIGCHLD, &signore, NULL);
  sigaction(SIGSTOP, &signore, NULL);
  sigaction(SIGTTOU, &signore, NULL);
  sigaction(SIGTTIN, &signore, NULL);
}

void signalHandler(int signal)
{
  switch(signal) {
  case SIGTERM:
  case SIGINT:
    stopMonitor();
    break;
  }
}

void setupMonitorParams(SMonitorParams *monitor_params)
{
  const char *mount_point = '\0';
  const char *monitor_file = '\0';
  const char *file_contents = '\0';
  const char *daemon = '\0';
  const char *daemon_args = '\0';
  const char *daemon_user = '\0';
  int timeout = 1;
  int wait_time = 15;

  // Read the mount point.
  if(configReadStr("mount_point", &mount_point) != CONFIG_TRUE) {
    // Error reading a mount point from the configuration file.
    syslog(LOG_ERR, "The option 'mount_point' was not found.");
    exit(1);
  }

  // Read the daemon location. 
  if(configReadStr("daemon", &daemon) != CONFIG_TRUE) {
    // Error 
    syslog(LOG_ERR, "The option 'daemon' was not found.");
    exit(1);
  }

  // Read the file to monitor.
  if(configReadStr("monitor_file", &monitor_file) != CONFIG_TRUE) {
    // Error
    syslog(LOG_ERR, "The option 'monitor_file' was not found.");
    exit(1);
  }

  // Read the file contents we use to check the file. 
  if(configReadStr("file_contents", &file_contents) != CONFIG_TRUE) {
    // Error 
    syslog(LOG_ERR, "The option 'file_contents' was not found.");
    exit(1);
  }

  // Read the darmon args.
  if(configReadStr("daemon_args", &daemon_args) != CONFIG_TRUE) {
    // Error 
    syslog(LOG_ERR, "The option 'daemon_args' was not found.");
    exit(1);
  }

  // Read the darmon user.
  if(configReadStr("daemon_user", &daemon_user) != CONFIG_TRUE) {
    // Error 
    syslog(LOG_ERR, "The option 'daemon_user' was not found.");
    exit(1);
  }

  // Read the timeout.
  if(configReadInt("timeout", &timeout) != CONFIG_TRUE) {
    // Error 
    syslog(LOG_ERR, "The option 'timeout' was not found.");
    exit(1);
  }

  // Read the wait time.
  if(configReadInt("wait_time", &wait_time) != CONFIG_TRUE) {
    // Error 
    syslog(LOG_ERR, "The option 'wait_time' was not found.");
    exit(1);
  }

  monitor_params->mount_point = copyString(mount_point);
  monitor_params->monitor_file = copyString(monitor_file);
  monitor_params->file_contents = copyString(file_contents);
  monitor_params->daemon = copyString(daemon);
  monitor_params->daemon_args = copyString(daemon_args);
  monitor_params->daemon_user = copyString(daemon_user);
  monitor_params->timeout = timeout;
  monitor_params->wait_time = wait_time;
}

void freeMonitorParams(SMonitorParams *monitor_params)
{
  if(monitor_params == NULL) {
    return;
  }

  free(monitor_params->mount_point);
  free(monitor_params->daemon);
}
