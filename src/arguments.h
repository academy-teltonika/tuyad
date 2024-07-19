#pragma once

#include <argp.h>
#include <stdbool.h>

extern const char doc[];
extern const char args_doc[];
extern struct argp_option options[];

struct arguments {
  char *product_id;
  char *device_id;
  char *device_secret;
  bool daemonize;
};

error_t parse_opt(int key, char *arg, struct argp_state *state);

struct arguments arguments_create(void);
