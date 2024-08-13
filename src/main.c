#include "tuya.h"
#include "log_level.h"
#include "arguments.h"
#include "ubus.h"

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

struct ubus_context *g_ubus_context;
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

  g_ubus_context = ubus_connect(NULL);
  if (g_ubus_context == NULL) {
    syslog(LOG_LEVEL_ERROR, "Failed to connect to ubus\n");
    return -1;
  }

  if (tuya_init(arguments) != OPRT_OK) {
    syslog(LOG_LEVEL_ERROR, "Failed to connect to Tuya cloud.");
    return -1;
  }
  syslog(LOG_LEVEL_INFO, "Tuya daemon started succesfully.");

  // Very coarse timer.
  time_t last_time = 0;
  time_t delta_time = INT_MAX;

  struct SystemInfo systemInfo = {0};
  while (g_running) {
    tuya_mqtt_loop(&g_tuya_context);

    if (delta_time >= CLOUD_REPORTING_INTERVAL_SEC) {
      last_time = time(NULL);

      get_ubus_system_info(&systemInfo, g_ubus_context);
      char *info_json_string = create_sysinfo_json(&systemInfo);
      tuyalink_thing_property_report(&g_tuya_context, NULL, info_json_string);
      syslog(LOG_LEVEL_INFO, "%s %s", "Sending sysinfo. JSON:", info_json_string);
      free(info_json_string);
    }

    delta_time = time(NULL) - last_time;
  }

  return 0;
}

void cleanup(void) {
  closelog();
  ubus_free(g_ubus_context);
  tuya_mqtt_disconnect(&g_tuya_context); // Possibly redundant. Need to read the documentation.
  tuya_mqtt_deinit(&g_tuya_context);
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
