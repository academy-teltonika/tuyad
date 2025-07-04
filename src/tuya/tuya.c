#include "tuya.h"
#include "tuya_error.h"
#include "tuya_action_esp.h"
#include "tuya_action_log.h"
#include "tuya_action_system.h"
#include "arguments.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
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

enum ParseTuyaActionResultType {
    PARSE_TUYA_ACTION_RESULT_OK,
    PARSE_TUYA_ACTION_RESULT_ERR_MALFORMED_JSON,
    PARSE_TUYA_ACTION_RESULT_ERR_METHOD_DOES_NOT_EXIST,
};

struct ParseTuyaActionResult {
    enum ParseTuyaActionResultType result;
    char *field;
};

static const char *ParseTuyaActionResult_message[] = {
    [PARSE_TUYA_ACTION_RESULT_OK] = "Success.",
    [PARSE_TUYA_ACTION_RESULT_ERR_MALFORMED_JSON] = "Action JSON is malformed.",
    [PARSE_TUYA_ACTION_RESULT_ERR_METHOD_DOES_NOT_EXIST] = "Method \\\"%s\\\" does not exist."};

// If result is an error, response_json_string will be the error message,
// otherwise, if result is an ok, response_json_string will be NULL.
static bool ParseTuyaActionResult_to_tuya_response_json_string(
    struct ParseTuyaActionResult result, char **response_json_string) {

  enum ParseTuyaActionResultType action_result = result.result;
  if (action_result == PARSE_TUYA_ACTION_RESULT_OK) {
    *response_json_string = NULL;
    return true;
  }

  char format_buf[256];
  switch (action_result) {
  case PARSE_TUYA_ACTION_RESULT_ERR_MALFORMED_JSON:
    *response_json_string = create_tuya_response_json(
        ParseTuyaActionResult_message
            [PARSE_TUYA_ACTION_RESULT_ERR_MALFORMED_JSON],
        false);
    return false;
  case PARSE_TUYA_ACTION_RESULT_ERR_METHOD_DOES_NOT_EXIST:
    snprintf(format_buf, sizeof(format_buf),
             ParseTuyaActionResult_message
                 [PARSE_TUYA_ACTION_RESULT_ERR_METHOD_DOES_NOT_EXIST],
             result.field);
    *response_json_string = create_tuya_response_json(format_buf, false);
    return false;
  default:
    assert(false); // should never happen.
  }

  return true;
}

// Use a hash-set if there are many action codes
// ParseTuyaActionResult.field gets set to reference of method name on invalid
// method.
static struct ParseTuyaActionResult
parse_tuya_action_type(cJSON *action_json, enum TuyaAction *action) {
  struct ParseTuyaActionResult result = {};
  cJSON *action_code_json = cJSON_GetObjectItem(action_json, "actionCode");
  if (action_code_json == NULL) {
    result.result = PARSE_TUYA_ACTION_RESULT_ERR_MALFORMED_JSON;
  }

  if (strcmp(action_code_json->valuestring, "log") == 0) {
    *action = TUYA_ACTION_LOG;
    result.result = PARSE_TUYA_ACTION_RESULT_OK;
    return result;
  }
  if (strcmp(action_code_json->valuestring, "read_sensor") == 0) {
    *action = TUYA_ACTION_ESP_READ_SENSOR;
    result.result = PARSE_TUYA_ACTION_RESULT_OK;
    return result;
  }
  if (strcmp(action_code_json->valuestring, "toggle_pin") == 0) {
    *action = TUYA_ACTION_ESP_TOGGLE_PIN;
    result.result = PARSE_TUYA_ACTION_RESULT_OK;
    return result;
  }
  if (strcmp(action_code_json->valuestring, "list_devices") == 0) {
    *action = TUYA_ACTION_ESP_LIST_DEVICES;
    result.result = PARSE_TUYA_ACTION_RESULT_OK;
    return result;
  }
  if (strcmp(action_code_json->valuestring, "sysinfo") == 0) {
    *action = TUYA_ACTION_SYSTEM_INFO;
    result.result = PARSE_TUYA_ACTION_RESULT_OK;
    return result;
  }

  result.result = PARSE_TUYA_ACTION_RESULT_ERR_METHOD_DOES_NOT_EXIST;
  result.field = action_code_json->valuestring;
  return result;
}

static void execute_tuya_action(struct tuya_mqtt_context *context,
                                const tuyalink_message_t *msg) {
  enum TuyaAction tuya_action;
  char *response_json_string;

  // TODO: Consolidate into one parse function.
  cJSON *action_json = cJSON_Parse(msg->data_string);

  struct ParseTuyaActionResult ret = parse_tuya_action_type(action_json, &tuya_action);
  if (!ParseTuyaActionResult_to_tuya_response_json_string(ret, &response_json_string)) {
    goto end;
  }

  switch (tuya_action) {
  case TUYA_ACTION_ESP_TOGGLE_PIN:
  case TUYA_ACTION_ESP_READ_SENSOR:
    execute_commesp_esp_pin_action(EspAction_from_TuyaAction(tuya_action),
                                   action_json, &response_json_string);
    break;
  case TUYA_ACTION_ESP_LIST_DEVICES:
    execute_commesp_list_devices(&response_json_string);
    break;
  case TUYA_ACTION_LOG:
    execute_log_to_file(action_json, &response_json_string);
    break;
  case TUYA_ACTION_SYSTEM_INFO:
    execute_system_get_info(&response_json_string);
    break;
  }

end:
  if (response_json_string == NULL) {
    response_json_string = create_tuya_response_json("Failed to construct tuya response.", false);
  }
  tuyalink_thing_property_report(&g_tuya_context, NULL, response_json_string);
  cJSON_Delete(action_json);
  free(response_json_string);
}

static void on_messages(tuya_mqtt_context_t *context, void *user_data,
                        const tuyalink_message_t *msg) {
  switch (msg->type) {
  case THING_TYPE_ACTION_EXECUTE:
    execute_tuya_action(context, msg);
  default:
    break;
  }
}

static void
on_connected(tuya_mqtt_context_t *context, void *user_data) {
    syslog(LOG_LEVEL_INFO, "Succesfully connected to Tuya cloud.");
}

static void
on_disconnect(tuya_mqtt_context_t *context, void *user_data) {
    syslog(LOG_LEVEL_CRITICAL, "Lost connection to Tuya cloud.");
}

int
tuya_init(struct arguments args) {
    log_set_quiet(true);
    int ret = OPRT_OK;

    const tuya_mqtt_config_t tuya_mqtt_config = {
        .host = "m1.tuyacn.com",
        .port = 8883,
        .cacert = (const uint8_t *) tuya_cacert_pem,
        .cacert_len = sizeof(tuya_cacert_pem),
        .device_id = args.device_id,
        .device_secret = args.device_secret,
        .keepalive = 100,
        .timeout_ms = 2000,
        .on_connected = on_connected,
        .on_disconnect = on_disconnect,
        .on_messages = on_messages       
    };
    ret = tuya_mqtt_init(&g_tuya_context, &tuya_mqtt_config);
    if (ret != OPRT_OK) {
        return ret;
    }

    ret = tuya_mqtt_connect(&g_tuya_context);

    return ret;
}
