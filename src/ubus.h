#pragma once
#include "tuya_action_esp.h"
#include <libubus.h>
#include <stdbool.h>

extern struct ubus_context *g_ubus_context;

struct SystemInfo *ubus_invoke_get_system_info(struct SystemInfo *systemInfo,
                                               struct ubus_context *ctx);

bool ubus_invoke_toggle_esp_pin(struct EspRequest request,
                                struct EspResponse *response);

bool ubus_invoke_list_esp_devices(struct EspDevices *devices);

void create_ubus_message_from_esp_request(struct blob_buf *request_buf,
                                          enum EspAction action,
                                          struct EspRequest request);

bool ubus_init();

void ubus_deinit();
