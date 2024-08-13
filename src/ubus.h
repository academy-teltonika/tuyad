#pragma once
#include <libubus.h>
#include <stdbool.h>
#include <esp.h>

struct SystemInfo {
	unsigned long long total;
	unsigned long long free;
	int uptime;
	int load[3];
	bool parsed_successfuly; // Should probably not have this here.
};


static void ubus_parse_system_info(struct ubus_request *req, int type, struct blob_attr *msg);

struct SystemInfo *get_ubus_system_info(struct SystemInfo *systemInfo, struct ubus_context *ctx);

bool ubus_send_toggle_esp_pin_request(struct ubus_context *ctx, struct EspRequest request,
                                      struct EspResponse *response);
