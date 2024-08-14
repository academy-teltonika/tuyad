#pragma once
#include <libubus.h>
#include <stdbool.h>
#include "esp.h"

extern struct ubus_context *g_ubus_context;

static void ubus_parse_system_info(struct ubus_request *req, int type, struct blob_attr *msg);

struct SystemInfo *ubus_get_system_info(struct SystemInfo *systemInfo, struct ubus_context *ctx);

bool ubus_invoke_toggle_esp_pin(struct EspRequest request, struct EspResponse *response);
