#include "arguments.h"

const char doc[] = "Tuya cloud communication daemon.";
const char args_doc[] = "-p <PROCUCT-ID> -d <DEVICE-ID> -s <DEVICE-SECRET>";
struct argp_option options[] = {
  {"verbose", 'p', "product-id", 0, "Tuya cloud product id."},
  {"verbose", 'd', "device-id", 0, "Tuya cloud device id."},
  {"verbose", 's', "device-secret", 0, "Tuya cloud device secret."},
  {0}
};

error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;

  switch (key) {
    case 'p':
      arguments->product_id = arg;
      break;
    case 'd':
      arguments->device_id = arg;
      break;
    case 's':
      arguments->device_secret = arg;
      break;
    case ARGP_KEY_ARG:
      argp_error(state, "Use the -d -p -s flags to specify device information.");
      return ARGP_KEY_ERROR;

    case ARGP_KEY_END:
      if (arguments->product_id == NULL ||
          arguments->device_id == NULL ||
          arguments->device_secret == NULL) {
        argp_error(state, "Not all required options (-d -p -s) were provided.");
        return ARGP_KEY_ERROR;
      }

    default:
      return ARGP_ERR_UNKNOWN;
  }

  return 0;
}

struct arguments arguments_create(void) {
  struct arguments arguments;
  arguments.product_id = NULL;
  arguments.device_id = NULL;
  arguments.device_secret = NULL;
  return arguments;
}
