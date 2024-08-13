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

bool parse_tuya_action(char* message_json, enum TuyaAction *action) {
    enum TuyaAction requested_action = -1;
    cJSON *json = cJSON_Parse(message_json);
    cJSON *inputParams_json = cJSON_GetObjectItem(json, "inputParams");
    cJSON *action_code_json = cJSON_GetObjectItem(inputParams_json, "actionCode");


    if (strcmp(action_code_json->valuestring, "toggle_pin") == 0) {
        requested_action = TUYA_ACTION_PIN_TOGGLE;
    } else if (strcmp(action_code_json->valuestring, "list") == 0) {
    } else {
        return false;
    }

    cJSON_Delete(json);
    *action = requested_action;
    return true;
}

void on_connected(tuya_mqtt_context_t *context, void *user_data);

void on_disconnect(tuya_mqtt_context_t *context, void *user_data);

void on_messages(tuya_mqtt_context_t *context, void *user_data, const tuyalink_message_t *msg);

char *parse_log_message_json(char *json_string);

enum FileOperationResult append_message_to_file(char *filepath, char *message);

void syslog_file_operation(enum FileOperationResult result, char *user_data);

int tuya_init(tuya_mqtt_context_t *context, struct arguments args) {
    log_set_quiet(true);
    int ret = OPRT_OK;

    ret = tuya_mqtt_init(context, &(const tuya_mqtt_config_t){
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

    ret = tuya_mqtt_connect(context);

    return ret;
}

void on_connected(tuya_mqtt_context_t *context, void *user_data) {
    syslog(LOG_LEVEL_INFO, "Succesfully connected to Tuya cloud.");
}

void on_disconnect(tuya_mqtt_context_t *context, void *user_data) {
    syslog(LOG_LEVEL_CRITICAL, "Lost connection to Tuya cloud.");
}

void on_messages(tuya_mqtt_context_t *context, void *user_data, const tuyalink_message_t *msg) {
    char* message;

    switch (msg->type) {
        case THING_TYPE_ACTION_EXECUTE:
            enum TuyaAction action;
            parse_tuya_action(msg->data_string, &action);
            switch (action) {
                case TUYA_ACTION_PIN_TOGGLE:
                    ubus_toggle_esp_pin(0, NULL, g_ubus_context);
                    break;
                default:
                    break;
            }

            break;

        default:
            break;
    }
}

char *create_sysinfo_json(struct SystemInfo* systemInfo) {
    char* json_string = NULL;
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
