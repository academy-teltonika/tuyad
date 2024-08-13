#include "esp.h"
#include <stdlib.h>
#include <libubus.h>

void EspRequest_init(struct EspRequest *request) {
	request->pin = -1;
	request->port = NULL;
	request->pin_power = false;
	request->sensor = NULL;
	request->model = NULL;
}

void EspRequest_free(struct EspRequest *request) {
	if (request->port != NULL) {
		free(request->port);
		request->port = NULL;
	}
	if (request->sensor != NULL) {
		free(request->sensor);
		request->sensor = NULL;
	}
	if (request->model != NULL) {
		free(request->model);
		request->model = NULL;
	}
}

struct EspResponse EspResponse_new(void) {
	struct EspResponse response = {.success = false, .parsed_successfuly = true, .message = NULL, .data = NULL};
	return response;
}

void EspResponse_free(struct EspResponse *response) {
	if (response->data != NULL) {
		free(response->data);
		response->data = NULL;
	}
	if (response->message != NULL) {
		free(response->message);
		response->message = NULL;
	}
}

void esp_create_ubus_request_blobmsg(struct blob_buf *request_buf, enum EspAction action, struct EspRequest request) {
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

