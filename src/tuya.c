#include "tuya.h"

#include "log_level.h"
#include "tuya_cacert.h"
#include "log.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <syslog.h>
#include <arguments.h>

#define TUYA_ACTION_FILE_PATH "/tmp/tuya_action.log"

extern struct ubus_context *g_ubus_context;
tuya_mqtt_context_t g_tuya_context;

enum TuyaAction {
    TUYA_ACTION_LIST_DEVICES,
    TUYA_ACTION_PIN_TOGGLE,
};

enum FileOperationResult {
    FILE_OPERATION_RESULT_OK,
    FILE_OPERATION_RESULT_ERROR_OPEN,
    FILE_OPERATION_RESULT_ERROR_WRITE,
    FILE_OPERATION_RESULT_ERROR_CLOSE
};

static const char *FILE_OPERATION_RESULT_MESSAGE[] = {
    "Sucess.",
    "Failed to open file.",
    "Failed to write to file.",
    "Failed to close file."
};

bool parse_tuya_action(cJSON *action_json, enum TuyaAction *action) {
    cJSON *action_code_json = cJSON_GetObjectItem(action_json, "actionCode");
    if (action_code_json == NULL) {
        return false;
    }

    enum TuyaAction requested_action;
    if (strcmp(action_code_json->valuestring, "toggle_pin") == 0) {
        requested_action = TUYA_ACTION_PIN_TOGGLE;
    } else if (strcmp(action_code_json->valuestring, "list") == 0) {
        requested_action = TUYA_ACTION_LIST_DEVICES;
    } else {
        return false;
    }

    *action = requested_action;
    return true;
}

bool parse_tuya_action_esp_request(cJSON *action_json, struct EspRequest *esp_request) {
    cJSON *input_params_json = cJSON_GetObjectItem(action_json, "inputParams");
    if (input_params_json == NULL) {
        return false;
    }
    cJSON *port_json = cJSON_GetObjectItem(input_params_json, "port");
    if (port_json != NULL) {
        esp_request->port = malloc(strlen(port_json->valuestring));
        if (esp_request->port == NULL) {
            return false;
        }
        strcpy(esp_request->port, port_json->valuestring);
    }

    cJSON *pin_json = cJSON_GetObjectItem(input_params_json, "pin");
    if (pin_json != NULL) {
        esp_request->pin = pin_json->valueint;
    }

    cJSON *sensor_json = cJSON_GetObjectItem(input_params_json, "sensor");
    if (sensor_json != NULL) {
        esp_request->sensor = malloc(strlen(sensor_json->valuestring));
        if (esp_request->sensor == NULL) {
            return false;
        }
        strcpy(esp_request->sensor, sensor_json->valuestring);
    }

    cJSON *model_json = cJSON_GetObjectItem(input_params_json, "model");
    if (model_json != NULL) {
        esp_request->model = malloc(strlen(model_json->valuestring));
        if (esp_request->model == NULL) {
            return false;
        }
        strcpy(esp_request->model, model_json->valuestring);
    }

    return true;
}

char *create_esp_action_response_json(struct EspResponse response) {
    char *json_string = NULL;
    char field_buffer[1024];
    cJSON *packet = cJSON_CreateObject();

    if (!response.parsed_successfuly) {
        cJSON_AddStringToObject(packet, "result", "err");
        cJSON_AddStringToObject(packet, "message", "Failed to parse ubus response.");
    }

    // TODO NULL checks?
    snprintf(field_buffer, sizeof(field_buffer), "%s", response.success ? "ok" : "err");
    if (cJSON_AddStringToObject(packet, "result", field_buffer) == NULL) goto end;
    snprintf(field_buffer, sizeof(field_buffer), "%s", response.message);
    if (cJSON_AddStringToObject(packet, "message", field_buffer) == NULL) goto end;
    snprintf(field_buffer, sizeof(field_buffer), "%s", response.data);
    if (cJSON_AddStringToObject(packet, "data", field_buffer) == NULL) goto end;

end:
    json_string = cJSON_Print(packet);
    cJSON_Delete(packet);
    cJSON_Minify(json_string);
    return json_string;
}

// Todo fix static
void on_connected(tuya_mqtt_context_t *context, void *user_data);

void on_disconnect(tuya_mqtt_context_t *context, void *user_data);

void on_messages(tuya_mqtt_context_t *context, void *user_data, const tuyalink_message_t *msg);

char *parse_log_message_json(char *json_string);

enum FileOperationResult append_message_to_file(char *filepath, char *message);

void syslog_file_operation(enum FileOperationResult result, char *user_data);

int tuya_init(struct arguments args) {
    log_set_quiet(true);
    int ret = OPRT_OK;

    ret = tuya_mqtt_init(&g_tuya_context, &(const tuya_mqtt_config_t){
                             .host = "m1.tuyacn.com",
                             .port = 8883,
                             .cacert = tuya_cacert_pem,
                             .cacert_len = sizeof(tuya_cacert_pem),
                             .device_id = args.device_id,
                             .device_secret = args.device_secret,
                             .keepalive = 100,
                             .timeout_ms = 2000,
                             .on_connected = on_connected,
                             .on_disconnect = on_disconnect,
                             .on_messages = on_messages
                         });
    if (ret != OPRT_OK) {
        return ret;
    }

    ret = tuya_mqtt_connect(&g_tuya_context);

    return ret;
}

void on_connected(tuya_mqtt_context_t *context, void *user_data) {
    syslog(LOG_LEVEL_INFO, "Succesfully connected to Tuya cloud.");
}

void on_disconnect(tuya_mqtt_context_t *context, void *user_data) {
    syslog(LOG_LEVEL_CRITICAL, "Lost connection to Tuya cloud.");
}

void on_messages(tuya_mqtt_context_t *context, void *user_data, const tuyalink_message_t *msg) {
    switch (msg->type) {
        case THING_TYPE_ACTION_EXECUTE:
            // TODO move out
            enum TuyaAction action;
            cJSON *action_json = cJSON_Parse(msg->data_string);
            parse_tuya_action(action_json, &action);

            struct EspRequest esp_request;
            parse_tuya_action_esp_request(action_json, &esp_request);
            struct EspResponse esp_response;
            char *esp_response_json = NULL;
            cJSON_Delete(action_json);

            switch (action) {
                case TUYA_ACTION_PIN_TOGGLE:
                    ubus_send_toggle_esp_pin_request(g_ubus_context, esp_request, &esp_response);
                    break;
                default:
                    break;
            }

            esp_response_json = create_esp_action_response_json(esp_response);
            tuyalink_thing_property_report(&g_tuya_context, NULL, esp_response_json);

            EspRequest_free(&esp_request);
            free(esp_response_json);
            break;

        default:
            break;
    }
}

char *create_sysinfo_json(struct SystemInfo *systemInfo) {
    char *json_string = NULL;
    char field_buffer[256];
    cJSON *packet = cJSON_CreateObject();

    if (systemInfo == NULL) {
        goto end;
    }

    snprintf(field_buffer, sizeof(field_buffer), "%u", systemInfo->uptime);
    if (cJSON_AddStringToObject(packet, "uptime", field_buffer) == NULL) goto end;
    snprintf(field_buffer, sizeof(field_buffer), "%llu", systemInfo->total);
    if (cJSON_AddStringToObject(packet, "totalram", field_buffer) == NULL) goto end;
    snprintf(field_buffer, sizeof(field_buffer), "%llu", systemInfo->free);
    if (cJSON_AddStringToObject(packet, "freeram", field_buffer) == NULL) goto end;

    cJSON *loads = cJSON_AddArrayToObject(packet, "loads");
    if (loads == NULL) goto end;
    for (int i = 0; i < 3; i++) {
        snprintf(field_buffer, sizeof(field_buffer), "%u", systemInfo->load[i]);
        cJSON *load = cJSON_CreateString(field_buffer);
        if (load == NULL) goto end;
        cJSON_AddItemToArray(loads, load);
    }

end:
    json_string = cJSON_Print(packet);
    cJSON_Delete(packet);
    cJSON_Minify(json_string);
    return json_string;
}

char *parse_log_message_json(char *json_string) {
    cJSON *json = cJSON_Parse(json_string);
    cJSON *inputParams_json = cJSON_GetObjectItem(json, "inputParams");
    cJSON *message_json = cJSON_GetObjectItem(inputParams_json, "message");

    char *message = malloc(strlen(message_json->valuestring) + 1);
    strcpy(message, message_json->valuestring);
    cJSON_Delete(json);

    return message;
}

enum FileOperationResult append_message_to_file(char *filepath, char *message) {
    FILE *file = fopen(filepath, "a");
    if (file == NULL) {
        return FILE_OPERATION_RESULT_ERROR_OPEN;
    }

    int count = strlen(message);
    int ret = fwrite(message, sizeof(char), count, file);
    if (ret != count) {
        fclose(file);
        return FILE_OPERATION_RESULT_ERROR_WRITE;
    }
    fwrite("\n", sizeof(char), 1, file); // maybe should check for error

    ret = fclose(file);
    if (ret == EOF) {
        return FILE_OPERATION_RESULT_ERROR_CLOSE;
    }

    return FILE_OPERATION_RESULT_OK;
}

void syslog_file_operation(enum FileOperationResult result, char *user_data) {
    switch (result) {
        case FILE_OPERATION_RESULT_OK:
            syslog(LOG_LEVEL_INFO, "Succesfully written action message to file.");
            break;
        case FILE_OPERATION_RESULT_ERROR_CLOSE:
            syslog(LOG_LEVEL_WARNING, "Failed to close file");
            break;
        case FILE_OPERATION_RESULT_ERROR_OPEN:
        case FILE_OPERATION_RESULT_ERROR_WRITE:
            syslog(LOG_LEVEL_ERROR, "Could not write action message to file. %s",
                   FILE_OPERATION_RESULT_MESSAGE[result]);
            break;
    }
}
