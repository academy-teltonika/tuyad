#include "tuya.h"
#include "tuya_action_esp.h"

#include "arguments.h"

#include <tuyalink_core.h>
#include <tuya_cacert.h>
#include <log.h>
#include <stdlib.h>
#include <tuya_error_code.h>

#include <syslog.h>
#include "log_level.h"

#define TUYA_ACTION_FILE_PATH "/tmp/tuya_action.log"

extern struct ubus_context *g_ubus_context;
tuya_mqtt_context_t g_tuya_context;


// Use a hash-set if there are many action codes
bool parse_tuya_action_type(cJSON *action_json, enum TuyaAction *action) {
    cJSON *action_code_json = cJSON_GetObjectItem(action_json, "actionCode");
    if (action_code_json == NULL) {
        return false;
    }

    if (strcmp(action_code_json->valuestring, "log") == 0) {
        *action = TUYA_ACTION_LOG;
        return true;
    }
    if (strcmp(action_code_json->valuestring, "read_sensor") == 0) {
        *action = TUYA_ACTION_ESP_READ_SENSOR;
        return true;
    }
    if (strcmp(action_code_json->valuestring, "toggle_pin") == 0) {
        *action = TUYA_ACTION_ESP_TOGGLE_PIN;
        return true;
    }
    if (strcmp(action_code_json->valuestring, "list_devices") == 0) {
        *action = TUYA_ACTION_ESP_LIST_DEVICES;
        return true;
    }

    return false;
}


static void execute_tuya_action(struct tuya_mqtt_context *context, const tuyalink_message_t *msg) {
    enum TuyaAction tuya_action;
    cJSON *action_json = cJSON_Parse(msg->data_string);
    parse_tuya_action_type(action_json, &tuya_action);
    char *response_json_string = NULL;

    switch (tuya_action) {
        case TUYA_ACTION_ESP_TOGGLE_PIN:
        case TUYA_ACTION_ESP_READ_SENSOR:
            execute_commesp_esp_action(EspAction_from_TuyaAction(tuya_action), action_json, &response_json_string);
            break;
        case TUYA_ACTION_ESP_LIST_DEVICES:
            execute_commesp_list_devices(&response_json_string);
            break;
        case TUYA_ACTION_LOG:
            break;
    }

    tuyalink_thing_property_report(&g_tuya_context, NULL, response_json_string);
    cJSON_Delete(action_json);
    free(response_json_string);
}

static void on_messages(tuya_mqtt_context_t *context, void *user_data, const tuyalink_message_t *msg) {
    switch (msg->type) {
        case THING_TYPE_ACTION_EXECUTE:
            execute_tuya_action(context, msg);
        default:
            break;
    }
}

static void on_connected(tuya_mqtt_context_t *context, void *user_data) {
    syslog(LOG_LEVEL_INFO, "Succesfully connected to Tuya cloud.");
}

static void on_disconnect(tuya_mqtt_context_t *context, void *user_data) {
    syslog(LOG_LEVEL_CRITICAL, "Lost connection to Tuya cloud.");
}

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
