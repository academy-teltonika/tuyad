#pragma once
#include <stdbool.h>
#include <libubus.h>

#define ESP_COMMUNICATION_DAEMON_NAME "espcommd"

extern struct ubus_context *g_ubus_context;

bool ubus_init();

void ubus_deinit();
