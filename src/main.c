#include "arguments.h"
#include "log_level.h"
#include "tuya.h"
#include "ubus.h"
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <syslog.h>
#include <time.h>
#include <tuya_error_code.h> // TODO
#include <tuyalink_core.h>   // TODO
#include <unistd.h>

#define SYSLOG_OPTIONS LOG_PID | LOG_NDELAY
#define CLOUD_REPORTING_INTERVAL_SEC 5

void configure_signal_handlers(void);
void termination_handler(int signum);
void cleanup(void);

extern struct ubus_context *g_ubus_context;
extern struct tuya_mqtt_context g_tuya_context;
bool g_running = true;

// Todo: probably move some stuff into init functions
int main(int argc, char **argv) {
  struct arguments arguments = arguments_create();
  struct argp argp = {options, parse_opt, args_doc, doc};
  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  atexit(cleanup);
  configure_signal_handlers();
  openlog(NULL, SYSLOG_OPTIONS, LOG_LOCAL0);

  if (!ubus_init()) {
    syslog(LOG_LEVEL_ERROR, "Failed to connect to ubus\n");
    return -1;
  }

  if (tuya_init(arguments) != OPRT_OK) {
    syslog(LOG_LEVEL_ERROR, "Failed to connect to Tuya cloud.");
    return -1;
  }

  syslog(LOG_LEVEL_INFO, "Tuya daemon started succesfully.");

  while (g_running) {
    tuya_mqtt_loop(&g_tuya_context);
  }

  return 0;
}

void cleanup(void) {
  closelog();
  // tuya_mqtt_disconnect(&g_tuya_context); // Possibly redundant. Need to read
  // the documentation.
  tuya_mqtt_deinit(&g_tuya_context);
  ubus_deinit();
  syslog(LOG_LEVEL_INFO, "Shutdown successful.");
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
