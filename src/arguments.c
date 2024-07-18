#include "arguments.h"

const char doc[] = "Tuya cloud communication daemon.";
const char args_doc[] = "PROCUCT-ID DEVICE-ID DEVICE-SECRET";
struct argp_option options[] = {
  {"verbose", 'D', 0, 0, "Don't launch as a daemon process"},
  {0}
};

error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;

  switch (key) {
    case 'D':
      arguments->daemonize = false;
      break;
    case ARGP_KEY_ARG:
      if (state->arg_num >= 3) {
        argp_usage(state);
      }
      arguments->args[state->arg_num] = arg;
      break;

    case ARGP_KEY_END:
      if (state->arg_num < 3) {
        argp_usage(state);
        break;
      }

    default:
      return ARGP_ERR_UNKNOWN;
  }

  return 0;
}

struct arguments arguments_create(void) {
  struct arguments arguments;
  arguments.daemonize = true;
  for (int i = 0; i < 3; i++) {
    arguments.args[i] = NULL;
  }
  return arguments;
}
