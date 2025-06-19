#include "cJSON.h"
#include <assert.h>
#include <stdio.h>
#include "ubus_action_system.h"
#include "tuya_error.h"

static char *create_sysinfo_json(struct SystemInfo *systemInfo) {
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

static char *create_ubus_error_json(enum UbusSystemActionResult result) {
  char* ptr;
  switch (result) {
  case UBUS_SYSTEM_ACTION_RESULT_OK:
  	assert(false); // You passed an OK as an error.
  case UBUS_SYSTEM_ACTION_RESULT_ERR_SYSTEM_NOT_FOUND:
    ptr = create_tuya_response_json(UbusSystemActionResult_messages[UBUS_SYSTEM_ACTION_RESULT_ERR_SYSTEM_NOT_FOUND], false);
    return ptr;
  case UBUS_SYSTEM_ACTION_RESULT_ERR_ACTION_FAILED:
    return create_tuya_response_json(UbusSystemActionResult_messages[UBUS_SYSTEM_ACTION_RESULT_ERR_ACTION_FAILED], false);
  case UBUS_SYSTEM_ACTION_RESULT_ERR_PARSE_FAILED:
    return create_tuya_response_json(UbusSystemActionResult_messages[UBUS_SYSTEM_ACTION_RESULT_ERR_PARSE_FAILED], false);
  }
  assert(false); // Never happens, but compiler complains without it.
}

void execute_system_get_info(char **response_json) {
    struct SystemInfo systemInfo = {};
    enum UbusSystemActionResult result = ubus_invoke_get_system_info(&systemInfo);

    if (result != UBUS_SYSTEM_ACTION_RESULT_OK) {
        *response_json = create_ubus_error_json(result);
    } else {
        *response_json = create_sysinfo_json(&systemInfo);
    }

}

