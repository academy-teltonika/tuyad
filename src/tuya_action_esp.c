#include "tuya_action_esp.h"
#include <tuyalink_core.h>
#include "tuya.h"
#include "esp.h"
#include "ubus.h"

#include <assert.h>

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

char *create_esp_action_json_string_from_esp_response(struct EspResponse response) {
    char *json_string = NULL;
    char field_buffer[1024];
    cJSON *packet = cJSON_CreateObject();

    if (!response.parsed_successfuly) {
        cJSON_AddStringToObject(packet, "result", "err");
        cJSON_AddStringToObject(packet, "message", "Failed to parse ubus response.");
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

void execute_esp_action(enum EspAction action, cJSON *action_json, char **esp_action_response_json_string) {
    struct EspRequest esp_request = EspRequest_new(action);
    parse_esp_request_from_tuya_action_json(action_json, &esp_request);
    cJSON_Delete(action_json);
    struct EspResponse esp_response = EspResponse_new();

    switch (action) {
        case ESP_ACTION_TOGGLE_PIN:
            ubus_invoke_toggle_esp_pin(esp_request, &esp_response); // TODO handle error
            break;
        default:
            break;
    }

    EspRequest_free(&esp_request);
    *esp_action_response_json_string = create_esp_action_json_string_from_esp_response(esp_response);
    EspResponse_free(&esp_response);
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

