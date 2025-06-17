#include "tuya_action_esp.h"
#include <tuyalink_core.h>
#include "ubus.h"
#include "ubus_action_esp.h"
#include "ubus_action_system.h"
#include "tuya_error.h"

#include <assert.h>

static bool parse_esp_request_from_tuya_action_json(cJSON *action_json,
                                             struct EspRequest *esp_request) {
  cJSON *input_params_json = cJSON_GetObjectItem(action_json, "inputParams");
  if (input_params_json == NULL) {
    return false;
  }

  cJSON *port_json = cJSON_GetObjectItem(input_params_json, "port");
  if (port_json != NULL) {
    esp_request->port =
        calloc(strlen(port_json->valuestring) + 1, sizeof(char));
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
    esp_request->pin_power =
        strcmp(pin_power_json->valuestring, "on") == 0 ? true : false;
  }

  cJSON *sensor_json = cJSON_GetObjectItem(input_params_json, "sensor");
  if (sensor_json != NULL) {
    esp_request->sensor =
        calloc(strlen(sensor_json->valuestring) + 1, sizeof(char));
    if (esp_request->sensor == NULL) {
      return false;
    }
    strcpy(esp_request->sensor, sensor_json->valuestring);
  }

  cJSON *model_json = cJSON_GetObjectItem(input_params_json, "model");
  if (model_json != NULL) {
    esp_request->model =
        calloc(strlen(model_json->valuestring) + 1, sizeof(char));
    if (esp_request->model == NULL) {
      return false;
    }
    strcpy(esp_request->model, model_json->valuestring);
  }

  return true;
}

// TODO: this can potentialy fail - add error handling.
static char *EspResponse_to_json_string(struct EspResponse response) {
  char *json_string = NULL;
  char field_buffer[1024];
  cJSON *packet = cJSON_CreateObject();

  if (!response.parsed_successfuly) {
    cJSON_AddStringToObject(packet, "result", "err");
    cJSON_AddStringToObject(packet, "message", "Ubus parsing error.");
    goto end;
  }

  snprintf(field_buffer, sizeof(field_buffer), "%s",
           response.success ? "ok" : "err");
  if (cJSON_AddStringToObject(packet, "result", field_buffer) == NULL)
    goto end;
  if (response.message != NULL) {
    snprintf(field_buffer, sizeof(field_buffer), "%s", response.message);
    if (cJSON_AddStringToObject(packet, "message", field_buffer) == NULL)
      goto end;
  }
  switch (response.tag) {
  case ESP_ACTION_READ_SENSOR: {
    struct DHTSensorReading *reading = response.sensor_reading;
    snprintf(field_buffer, sizeof(field_buffer), "%f", reading->temperature);
    if (cJSON_AddStringToObject(packet, "temperature", field_buffer) == NULL)
      goto end;
    snprintf(field_buffer, sizeof(field_buffer), "%f", reading->humidity);
    if (cJSON_AddStringToObject(packet, "humidity", field_buffer) == NULL)
      goto end;
    }
    default:
    break;
    }

end:
	json_string = cJSON_Print(packet);
	cJSON_Delete(packet);
	cJSON_Minify(json_string);
	return json_string;
}

static char *EspDevices_to_json_string(struct EspDevices *device_list) {
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

static char *create_ubus_error_json(enum UbusCommespActionResult result) {
  switch (result) {
  case UBUS_COMMESP_ACTION_RESULT_OK:
  	assert(false); // You passed a OK as an error.
  case UBUS_COMMESP_ACTION_RESULT_ERR_COMMESPD_NOT_FOUND:
    return create_tuya_response_json(UbusCommespActionResult_messages[UBUS_COMMESP_ACTION_RESULT_ERR_COMMESPD_NOT_FOUND], false);
  case UBUS_COMMESP_ACTION_RESULT_ERR_COMMESPD_ACTION_FAILED:
    return create_tuya_response_json(UbusCommespActionResult_messages [UBUS_COMMESP_ACTION_RESULT_ERR_COMMESPD_ACTION_FAILED], false);
  case UBUS_COMMESP_ACTION_RESULT_ERR_PARSE_FAILED:
    return create_tuya_response_json(UbusCommespActionResult_messages [UBUS_COMMESP_ACTION_RESULT_ERR_PARSE_FAILED], false);
  }
  assert(false); // Never happens, but compiler complains without it.
}

void execute_commesp_esp_pin_action(enum EspAction action,
                                    cJSON *tuya_action_json,
                                    char **esp_action_response_json_string) {
	enum UbusCommespActionResult result = UBUS_COMMESP_ACTION_RESULT_OK;
  struct EspRequest esp_request = EspRequest_new(action);
  parse_esp_request_from_tuya_action_json(tuya_action_json,
                                          &esp_request); // TODO parse checking
  struct EspResponse esp_response = EspResponse_new(action);

  switch (action) {
  case ESP_ACTION_TOGGLE_PIN:
		result = ubus_invoke_esp_toggle_pin(&esp_request, &esp_response);
    break;
  case ESP_ACTION_READ_SENSOR:
  	esp_response.sensor_reading = malloc(sizeof(struct DHTSensorReading));
		result = ubus_invoke_esp_read_sensor(&esp_request, &esp_response);
		break;
  default:
  	break;
  }

  if (result != UBUS_COMMESP_ACTION_RESULT_OK) {
  	*esp_action_response_json_string = create_ubus_error_json(result);
  } else {
  	*esp_action_response_json_string = EspResponse_to_json_string(esp_response);
  }

	EspRequest_free(&esp_request);
	EspResponse_free(&esp_response);
}

void execute_commesp_list_devices(char **commesp_resposne_json) {
	struct EspDevices device_list = EspDevices_new();
	enum UbusCommespActionResult result = ubus_invoke_list_esp_devices(&device_list);

	if (result != UBUS_COMMESP_ACTION_RESULT_OK) {
		*commesp_resposne_json = create_ubus_error_json(result);
	} else {
		*commesp_resposne_json = EspDevices_to_json_string(&device_list);
	}

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

struct EspResponse EspResponse_new(enum EspAction action) {
	struct EspResponse response = {.success = false, .parsed_successfuly = true, .message = NULL, .sensor_reading = NULL, .tag = action};
	return response;
}

void EspResponse_free(struct EspResponse *response) {
  switch (response->tag) {
  case ESP_ACTION_READ_SENSOR:
  if (response->sensor_reading != NULL) {
    free(response->sensor_reading);
    response->sensor_reading = NULL;
  }
    break;
  default:
    break;
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
