#pragma once

#include <stdbool.h>

extern const char* UbusCommespActionResult_messages[];

enum UbusCommespActionResult {
  UBUS_COMMESP_ACTION_RESULT_OK,
  UBUS_COMMESP_ACTION_RESULT_ERR_COMMESPD_NOT_FOUND,
  UBUS_COMMESP_ACTION_RESULT_ERR_COMMESPD_ACTION_FAILED,
  UBUS_COMMESP_ACTION_RESULT_ERR_PARSE_FAILED,
};

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

struct DHTSensorReading {
	float temperature;
	float humidity;
};

struct EspResponse {
	bool success;
	char *message;
	enum EspAction tag;
	union {
		struct DHTSensorReading *sensor_reading;
	};
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

struct EspResponse EspResponse_new(enum EspAction action);
void EspResponse_free(struct EspResponse *response);

struct EspDevices EspDevices_new(void);
void EspDevices_free(struct EspDevices *device_list);

enum UbusCommespActionResult ubus_invoke_esp_toggle_pin(struct EspRequest *request, struct EspResponse *response);
enum UbusCommespActionResult ubus_invoke_list_esp_devices(struct EspDevices *devices);
enum UbusCommespActionResult ubus_invoke_esp_read_sensor(struct EspRequest *request, struct EspResponse *response);
