#pragma once
#include <stdbool.h>

enum EspAction {
	ESP_ACTION_TOGGLE_PIN,
	ESP_ACTION_READ_SENSOR,
};


struct EspRequest {
	char *port;
	int pin;

	union {
		bool pin_power;

		struct {
			char *sensor;
			char *model;
		};
	};
};

struct EspResponse {
	bool success;
	char *message;
	char *data;
	bool parsed_successfuly; // Should probably not have this here.
};

void EspRequest_init(struct EspRequest *request);

void EspRequest_free(struct EspRequest *request);

struct EspResponse EspResponse_new(void);

void EspResponse_free(struct EspResponse *response);

void esp_create_ubus_request_blobmsg(struct blob_buf *request_buf, enum EspAction action, struct EspRequest request);
