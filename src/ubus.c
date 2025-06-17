#include "ubus.h"

#include "ubus_parsing.h"

struct ubus_context *g_ubus_context;

bool ubus_init() {
  g_ubus_context = ubus_connect(NULL);
  return g_ubus_context != NULL;
}

void ubus_deinit() { ubus_free(g_ubus_context); }
