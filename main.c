#include "tuya.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <unistd.h>

static char doc[] = "Tuya cloud communication daemon.";
static char args_doc[] = "PROCUCT-ID DEVICE-ID DEVICE-SECRET";
static error_t parse_opt (int key, char *arg, struct argp_state *state);
struct arguments
{
  char* args[3];
};

int main(int argc, char **argv) {
    static struct argp argp = {0, parse_opt, args_doc, doc};
    struct arguments arguments;
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    tuya_mqtt_context_t context;
    int ret = tuya_init(&context, arguments.args);
    assert(ret == OPRT_OK);

    while (true) {
        tuya_mqtt_loop(&context);

        char* info = create_sysinfo_json();
        tuyalink_thing_property_report(&context, NULL, info);
        free(info);
    }
}

static error_t parse_opt (int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;

  switch (key)
    {
    case ARGP_KEY_ARG:
      if (state->arg_num >= 3) {
        argp_usage (state);
      }
      arguments->args[state->arg_num] = arg;
      break;

    case ARGP_KEY_END:
      if (state->arg_num < 3) {
        argp_usage (state);
        break;
      }

    default:
      return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

#include <syslog.h>

void openlogw() {
    // setlogmask (LOG_UPTO (LOG_NOTICE));
    openlog(NULL, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);
}

void syslogw() {
  //syslog(LOG_INFO, "PID=%s   DID=%s   DS=%s\n", arguments.args[0], arguments.args[1], arguments.args[2]);
  syslog(LOG_WARNING, "Exiting from the program");
}

void closelogw() {
    closelog();
}
