#include "ubus.h"

#include "ubus_parsing.h"
#include "log_level.h"
#include "tuya_action_log.h"
#include "tuya_action_esp.h"
#include <syslog.h>

struct ubus_context *g_ubus_context;


struct SystemInfo *ubus_invoke_get_system_info(struct SystemInfo *systemInfo, struct ubus_context *ctx) {
	unsigned int id;
	if (ubus_lookup_id(ctx, "system", &id) ||
	    ubus_invoke(ctx, id, "info", NULL, ubus_parse_system_info, systemInfo, 3000)) {
		syslog(LOG_LEVEL_ERROR, "Failed to request system info from system.");
		return NULL;
	}

	if (!systemInfo->parsed_successfuly) {
		syslog(LOG_LEVEL_DEBUG, "Failed to parse system info from ubus.");
		return NULL;
	}

	return systemInfo;
}


bool ubus_invoke_toggle_esp_pin(struct EspRequest request, struct EspResponse *response) {
	bool success = true;
	unsigned int id;
	if (ubus_lookup_id(g_ubus_context, "commesp", &id) != 0) {
		return false;
	}

	struct blob_buf ubus_message = {};
	blob_buf_init(&ubus_message, 0);
	create_ubus_message_from_esp_request(&ubus_message, ESP_ACTION_TOGGLE_PIN, request);
	if (ubus_invoke(g_ubus_context,
	                id,
	                request.pin_power ? "on" : "off",
	                ubus_message.head,
	                ubus_parse_commesp_esp_action_response,
	                response,
	                3000) != 0) {
		success = false;
	}

	blob_buf_free(&ubus_message);
	return success;
}

bool ubus_invoke_list_esp_devices(struct EspDevices *devices) {
	unsigned int id;
	if (ubus_lookup_id(g_ubus_context, "commesp", &id) != 0) {
		return false;
	}

	if (ubus_invoke(g_ubus_context, id, "devices", NULL, ubus_parse_commesp_devices, devices, 3000) != 0) {
		return false;
	}

	return true;
}

bool ubus_init() {
	g_ubus_context = ubus_connect(NULL);
	return g_ubus_context != NULL;
}

void ubus_deinit() {
	ubus_free(g_ubus_context);
}

void create_ubus_message_from_esp_request(struct blob_buf *request_buf, enum EspAction action,
                                          struct EspRequest request) {
	if (request.port != NULL) {
		blobmsg_add_string(request_buf, "port", request.port);
	}
	blobmsg_add_u32(request_buf, "pin", request.pin);

	switch (action) {
		case ESP_ACTION_READ_SENSOR:
			if (request.sensor != NULL) {
				blobmsg_add_string(request_buf, "sensor", request.sensor);
			}
			if (request.model != NULL) {
				blobmsg_add_string(request_buf, "model", request.model);
			}
			break;
		case ESP_ACTION_TOGGLE_PIN:
			break;
	}
}
