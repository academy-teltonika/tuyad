#include "tuya.h"
#include "log_level.h"
#include "arguments.h"
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <syslog.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#define SYSLOG_OPTIONS LOG_PID | LOG_NDELAY
#define CLOUD_REPORTING_INTERVAL_SEC 5

void configure_signal_handlers(void);

void termination_handler(int signum);

void cleanup();

bool daemonize();

tuya_mqtt_context_t g_context;
bool g_running = true;

int main(int argc, char **argv) {
  struct arguments arguments = arguments_create();
  struct argp argp = {options, parse_opt, args_doc, doc};
  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  // TODO: Print out a success/error message to the console if the daemon started correctly.
  if (arguments.daemonize) {
    if (!daemonize()) {
      syslog(LOG_LEVEL_ERROR, "Failed to start Tuya daemon.");
      return -1;
    }
  }

  configure_signal_handlers();
  openlog(NULL, SYSLOG_OPTIONS, LOG_LOCAL0);
  atexit(cleanup);

  if (tuya_init(&g_context, arguments) != OPRT_OK) {
    syslog(LOG_LEVEL_ERROR, "Failed to connect to Tuya cloud.");
    return -1;
  }
  syslog(LOG_LEVEL_INFO, "Tuya daemon started succesfully.");

  // Very coarse timer.
  time_t last_time = 0;
  time_t delta_time = INT_MAX;

  while (g_running) {
    tuya_mqtt_loop(&g_context);

    if (delta_time >= CLOUD_REPORTING_INTERVAL_SEC) {
      last_time = time(NULL);
      char *info_json_string = create_sysinfo_json();
      tuyalink_thing_property_report(&g_context, NULL, info_json_string);
      syslog(LOG_LEVEL_INFO, "%s %s", "Sending sysinfo. JSON:", info_json_string);
      free(info_json_string);
    }

    delta_time = time(NULL) - last_time;
  }

  return 0;
}

void cleanup(void) {
  closelog();
  tuya_mqtt_disconnect(&g_context); // Possibly redundant. Need to read the documentation.
  tuya_mqtt_deinit(&g_context);
}

void configure_signal_handlers(void) {
  struct sigaction action;
  action.sa_handler = termination_handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;

  sigaction(SIGINT, &action, NULL);
  sigaction(SIGQUIT, &action, NULL);
  sigaction(SIGTERM, &action, NULL);
  sigaction(SIGHUP, &action, NULL);
}

void termination_handler(int signum) {
  g_running = false;
  syslog(LOG_LEVEL_INFO, "Terminating from signal: %d", signum);
}

bool daemonize() {
  switch (fork()) {
    case 0:
      break;
    case -1:
      return false;
    default:
      exit(0);
  }

  if (setsid() == -1) {
    return false;
  }

  switch (fork()) {
    case 0:
      break;
    case -1:
      return false;
    default:
      exit(0);
  }

  umask(0);
  chdir("/");

  close(STDIN_FILENO);
  int fd = open("/dev/null", O_RDWR);
  if (fd != STDIN_FILENO)
    return false;
  if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
    return false;
  if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
    return false;

  return true;
}
