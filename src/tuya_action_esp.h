#pragma once
#include "tuya.h"
#include <cJSON.h>

enum EspAction {
	ESP_ACTION_TOGGLE_PIN,
	ESP_ACTION_READ_SENSOR,
};

struct EspRequest {
	char *port;
	int pin;

	enum EspAction tag;

	union {
		struct {
			bool pin_power;
		};

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
	bool parsed_successfuly;
};

struct EspDevice {
	char *port;
	char *vid;
	char *pid;
};

struct EspDevices {
	struct EspDevice *devices;
	int count;
};

struct EspRequest EspRequest_new(enum EspAction action);

void EspRequest_free(struct EspRequest *request);

struct EspResponse EspResponse_new(void);

void EspResponse_free(struct EspResponse *response);

void execute_commesp_esp_action(enum EspAction action, cJSON *tuya_action_json, char **esp_action_response_json_string);

void execute_commesp_list_devices(char **commesp_resposne_json);

enum EspAction EspAction_from_TuyaAction(enum TuyaAction tuya_action);

struct EspDevices EspDevices_new(void);

void EspDevices_free(struct EspDevices *device_list);
