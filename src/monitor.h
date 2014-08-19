#ifndef _MONITOR_H_
#define _MONITOR_H_

struct SMonitorParams {
  char *mount_point;
  char *monitor_file;
  char *file_contents;
  char *daemon;
  char *daemon_args;
  char *daemon_user;
  int timeout;
  int wait_time;
};
typedef struct SMonitorParams SMonitorParams;


void startMonitor(const SMonitorParams *params);
void stopMonitor();

// Create a child process that will monitor for the given timeout
// ... a file to determine if it exists and if the filesystem is
// ... not stale.
// Return 1 if file exists and is readable. Return 0 otherwise.
int monitorFile(const char *path, const char *file, const char *contents, int timeout);
// Open a file and verify that it's contents match what is specified.
// ... signal the status of the file with an exit code.
void verifyFile(const char *path, const char *file, const char *contents);

// Wait for a child process with PID child_pid for timeout seconds.
int waitForChild(int child_pid, int timeout);

// Execute the daemon.
void execDaemon(const SMonitorParams *params);

#endif
