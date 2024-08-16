#include "tuya_action_esp.h"
#include <tuyalink_core.h>
#include "tuya.h"
#include "ubus.h"

#include <assert.h>

// If more complex ubus error handling is required, this should be refactored into a function which forms the error message
#define UBUS_PARSE_RESPONSE_ERROR_MESSAGE "{\"result\":\"err\", \"message\":\"Failed to parse ubus response.\"}"
#define UBUS_FAILURE_ERROR_MESSAGE "{\"result\":\"err\", \"message\":\"Ubus failure.\"}"

bool parse_esp_request_from_tuya_action_json(cJSON *action_json, struct EspRequest *esp_request) {
	cJSON *input_params_json = cJSON_GetObjectItem(action_json, "inputParams");
	if (input_params_json == NULL) {
		return false;
	}
	cJSON *port_json = cJSON_GetObjectItem(input_params_json, "port");
	if (port_json != NULL) {
		esp_request->port = calloc(strlen(port_json->valuestring) + 1, sizeof(char));
		if (esp_request->port == NULL) {
			return false;
		}
		strcpy(esp_request->port, port_json->valuestring);
	}

	cJSON *pin_json = cJSON_GetObjectItem(input_params_json, "pin");
	if (pin_json != NULL) {
		esp_request->pin = pin_json->valueint;
	}

	cJSON *pin_power_json = cJSON_GetObjectItem(input_params_json, "power");
	if (pin_power_json != NULL) {
		esp_request->pin_power = strcmp(pin_power_json->valuestring, "on") == 0 ? true : false;
	}

	cJSON *sensor_json = cJSON_GetObjectItem(input_params_json, "sensor");
	if (sensor_json != NULL) {
		esp_request->sensor = calloc(strlen(sensor_json->valuestring) + 1, sizeof(char));
		if (esp_request->sensor == NULL) {
			return false;
		}
		strcpy(esp_request->sensor, sensor_json->valuestring);
	}

	cJSON *model_json = cJSON_GetObjectItem(input_params_json, "model");
	if (model_json != NULL) {
		esp_request->model = calloc(strlen(model_json->valuestring) + 1, sizeof(char));
		if (esp_request->model == NULL) {
			return false;
		}
		strcpy(esp_request->model, model_json->valuestring);
	}

	return true;
}

char *EspResponse_to_json_string(struct EspResponse response) {
	char *json_string = NULL;
	char field_buffer[1024];
	cJSON *packet = cJSON_CreateObject();

	if (!response.parsed_successfuly) {
		cJSON_AddStringToObject(packet, "result", "err");
		cJSON_AddStringToObject(packet, "message", "Ubus parsing error.");
		goto end;
	}

	snprintf(field_buffer, sizeof(field_buffer), "%s", response.success ? "ok" : "err");
	if (cJSON_AddStringToObject(packet, "result", field_buffer) == NULL) goto end;
	if (response.message != NULL) {
		snprintf(field_buffer, sizeof(field_buffer), "%s", response.message);
		if (cJSON_AddStringToObject(packet, "message", field_buffer) == NULL) goto end;
	}
	if (response.data != NULL) {
		snprintf(field_buffer, sizeof(field_buffer), "%s", response.data);
		if (cJSON_AddStringToObject(packet, "data", field_buffer) == NULL) goto end;
	}


end:
	json_string = cJSON_Print(packet);
	cJSON_Delete(packet);
	cJSON_Minify(json_string);
	return json_string;
}

char *EspDevices_to_json_string(struct EspDevices *device_list) {
	char *json_string = NULL;
	char field_buffer[1024];
	cJSON *packet = cJSON_CreateObject();

	if (device_list->count < 0) {
		cJSON_AddStringToObject(packet, "result", "err");
		cJSON_AddStringToObject(packet, "message", "Ubus parsing error.");
		goto end;
	}

	cJSON_AddStringToObject(packet, "result", "ok");
	switch (device_list->count) {
		case 0:
			cJSON_AddStringToObject(packet, "message", "No devices found.");
			break;
		case 1:
			cJSON_AddStringToObject(packet, "message", "Found 1 device.");
			break;
		default:
			snprintf(field_buffer, sizeof(field_buffer), "Found %d devices.", device_list->count);
			cJSON_AddStringToObject(packet, "message", field_buffer);
			break;
	}

	// TODO json construction error handling
	cJSON *devices = cJSON_AddArrayToObject(packet, "devices");
	for (int i = 0; i < device_list->count; i++) {
		cJSON *device = cJSON_CreateObject();
		cJSON_AddStringToObject(device, "port", device_list->devices[i].port);
		cJSON_AddStringToObject(device, "vid", device_list->devices[i].vid);
		cJSON_AddStringToObject(device, "pid", device_list->devices[i].pid);
		cJSON_AddItemToArray(devices, device);
	}

end:
	json_string = cJSON_Print(packet);
	cJSON_Delete(packet);
	cJSON_Minify(json_string);
	return json_string;
}

void execute_commesp_esp_action(enum EspAction action, cJSON *tuya_action_json,
                                char **esp_action_response_json_string) {
	struct EspRequest esp_request = EspRequest_new(action);
	parse_esp_request_from_tuya_action_json(tuya_action_json, &esp_request);
	struct EspResponse esp_response = EspResponse_new();

	switch (action) {
		case ESP_ACTION_TOGGLE_PIN:
			if (!ubus_invoke_toggle_esp_pin(esp_request, &esp_response)) {
				*esp_action_response_json_string = malloc(strlen(UBUS_FAILURE_ERROR_MESSAGE) + 1);
				if (*esp_action_response_json_string == NULL) goto end;
				strcpy(*esp_action_response_json_string, UBUS_FAILURE_ERROR_MESSAGE);
				goto end;
			}
			break;
		default:
			break;
	}

	*esp_action_response_json_string = EspResponse_to_json_string(esp_response);
end:
	EspRequest_free(&esp_request);
	EspResponse_free(&esp_response);
}

void execute_commesp_list_devices(char **commesp_resposne_json) {
	struct EspDevices device_list = EspDevices_new();
	if (!ubus_invoke_list_esp_devices(&device_list)) {
		*commesp_resposne_json = malloc(strlen(UBUS_FAILURE_ERROR_MESSAGE) + 1);
		if (*commesp_resposne_json == NULL) goto end;
		strcpy(*commesp_resposne_json, UBUS_FAILURE_ERROR_MESSAGE);
		goto end;
	}

	*commesp_resposne_json = EspDevices_to_json_string(&device_list);
end:
	EspDevices_free(&device_list);
}

enum EspAction EspAction_from_TuyaAction(enum TuyaAction tuya_action) {
	switch (tuya_action) {
		case TUYA_ACTION_ESP_TOGGLE_PIN:
			return ESP_ACTION_TOGGLE_PIN;
		case TUYA_ACTION_ESP_READ_SENSOR:
			return ESP_ACTION_READ_SENSOR;
		default:
			assert(false); // If this happens - you've screwed up
	}
}

struct EspRequest EspRequest_new(enum EspAction action) {
	struct EspRequest request;
	request.pin = -1;
	request.port = NULL;
	request.pin_power = false;
	request.sensor = NULL;
	request.model = NULL;
	request.tag = action;
	return request;
}

void EspRequest_free(struct EspRequest *request) {
	if (request->port != NULL) {
		free(request->port);
		request->port = NULL;
	}
	if (request->tag == ESP_ACTION_READ_SENSOR) {
		if (request->sensor != NULL) {
			free(request->sensor);
			request->sensor = NULL;
		}
		if (request->model != NULL) {
			free(request->model);
			request->model = NULL;
		}
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

struct EspDevices EspDevices_new(void) {
	struct EspDevices devices = {.count = -1, .devices = NULL};
	return devices;
}

void EspDevices_free(struct EspDevices *device_list) {
	for (int i = 0; i < device_list->count; i++) {
		free(device_list->devices[i].port);
		free(device_list->devices[i].vid);
		free(device_list->devices[i].pid);
	}
	if (device_list->count > 0) {
		free(device_list->devices);
	}
}
