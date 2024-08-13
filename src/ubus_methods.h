#pragma once
#include <libubus.h>
#include <stdbool.h>

struct SystemInfo {
	unsigned long long total;
	unsigned long long free;
	int uptime;
	int load[3];
	bool parsed_successfuly; // Should probably not have this here.
};

struct EspResponse {
	bool success;
	char *message;
	char *data;
	bool parsed_successfuly; // Should probably not have this here.
};

static void parse_ubus_system_info(struct ubus_request *req, int type, struct blob_attr *msg);
struct SystemInfo* get_ubus_system_info(struct SystemInfo *systemInfo, struct ubus_context *ctx);
char* ubus_toggle_esp_pin(int pin, char *port, struct ubus_context *ctx);
