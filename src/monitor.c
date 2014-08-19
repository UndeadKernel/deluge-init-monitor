#include "monitor.h"

#include <syslog.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pwd.h>


// Flag that tells the monitor to stop.
static int stop_monitor = 0;


void startMonitor(const SMonitorParams *params)
{
  int file_ok = 0;

  // The main daemon payload.
  while(stop_monitor == 0) {
    // Start monitoring the file.
    file_ok = monitorFile(params->mount_point, 
			  params->monitor_file, 
			  params->file_contents,
			  params->timeout);

    // Was the file read correctly?
    if(file_ok == 1) {
      // It was, so switch to the deluge daemon.

      execDaemon(params);
      // This call does not return.
    }

    // Wait some time until we check again.
    sleep(params->wait_time);
  }

  syslog(LOG_INFO, "Stopping deluge monitor.");
}

void stopMonitor()
{
  // Signal other components of the monitor to stop.
  stop_monitor = 1;
}

int monitorFile(const char *path, const char *file, const char *contents, int timeout)
{
  int pid;
  int file_ok;

  // Create a child process that will try to access the file.
  pid = fork();
  if(pid == 0) {
    // The Child process.
    verifyFile(path, file, contents);
    // We should not reach this point.
    exit(1);
  }
  else if(pid > 0) {
    // The Parent process.
    file_ok = waitForChild(pid, timeout);
    return file_ok;
  }
  else {
    // Error when forking.
    syslog(LOG_ERR, "Error encountered when child process was forked.");
    exit(1);
  }

  return 0;
}

void verifyFile(const char *path, const char *file, const char *contents)
{
  int fd;
  int file_path_length = 0;
  char *file_path = 0;
  char *buf = 0;
  int buf_length = 0;

  // Reset the error number.
  errno = 0;

  // Get the file path.
  file_path_length = strlen(path) + strlen(file) + 1;

  // Create the path char array.
  file_path = (char *)malloc(sizeof(char) * file_path_length);

  // Add the path string
  snprintf(file_path, file_path_length, "%s%s", path, file);
  syslog(LOG_INFO, "File being monitored: %s", file_path);

  // Read the file.
  fd = open(file_path, O_RDONLY);
  free(file_path);

  if(fd == -1) {
    syslog(LOG_ERR, strerror(errno));
    // File not read successfully.
    exit(1);
  }
  else {
    // See if the contents match.
    buf_length = strlen(contents) + 1;
    buf = (char*)malloc(sizeof(char) * buf_length);

    // Read a string the same size as what we are looking for.
    if(read(fd, buf, buf_length - 1) == buf_length - 1) {
      buf[buf_length - 1] = '\0';
      // Do the strings match?
      if(strcmp(buf, contents) == 0) {
	// The contents do match.
	free(buf);
	close(fd);
	// File read correctly.
	exit(0);
      }
      else {
	printf("Content param does not match file content.");
      }
    }
    else {
      printf("Content read size mismatch.");
    }
  }

  // If there were any errors, print them.
  if(errno != 0) {
    syslog(LOG_ERR, strerror(errno));
  }

  free(buf);
  close(fd);
  // Something did not go according to the plan.
  exit(1);
}

int waitForChild(int child_pid, int timeout)
{
  int status;
  int pid;
  
  while(timeout != 0) {
    pid = waitpid(child_pid, &status, WNOHANG);
    if(pid == 0) {
      // Still waiting for a child.
      sleep(1);
      timeout--;
    }
    else if(pid == -1) {
      // Error
      syslog(LOG_ERR, strerror(errno));
      exit(1);
    }
    else {
      // The child exited.
      if(WIFEXITED(status)) {
	// Child was able to call exit().
	if(WEXITSTATUS(status) == 0) {	
	  printf("File read successfully!\n");
	  return 1;
	}
      }
      printf("File NOT read successfully.\n");
      return 0;
    }
  }

  // The child did not finish and the timeout was hit.
  kill(child_pid, 9);
  printf("Timeout reading the file!\n");
  return 0;
}

void execDaemon(const SMonitorParams *params)
{
  int i;
  char *argv[20];
  char *arg;
  struct passwd *pw;

  // Get the UID of the user to execute the daemon.
  errno = 0;
  pw = getpwnam(params->daemon_user);
  if(errno != 0) {
    syslog(LOG_ERR, "Cannot use user %s: %s", params->daemon_user, strerror(errno));
    exit(1);
  }
  else if(pw == NULL) {
    syslog(LOG_ERR, "User %s not found in the system.", params->daemon_user);
    exit(1);
  }
  

  // Generate the arguments vector.
  i = 0;
  argv[i++] = "deluged";
  arg = strtok(params->daemon_args, " ");
  while(arg != NULL) {
    argv[i++] = arg;
    arg = strtok(NULL, " ");
  }
  argv[i] = NULL;

  // Switch to the user ID.
  if(setuid(pw->pw_uid)) {
    // Error switching user.
    syslog(LOG_ERR, "Not able to switch to user %s: %s", params->daemon_user, strerror(errno));
    exit(1);
  }

  // Now execute the deluge daemon.
  syslog(LOG_INFO, "Starting the deluged daemon.");
  execv(params->daemon, argv);
  syslog(LOG_ERR, "Execution of the daemon failed: %s", strerror(errno));
  exit(1);

  return;
}
