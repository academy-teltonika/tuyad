#include "tuya.h"

#include "tuya_cacert.h"
// #include "system_interface.h"
// #include "mqtt_client_interface.h"
#include "log.h"

#include <sys/sysinfo.h>

void on_messages(tuya_mqtt_context_t* context, void* user_data, const tuyalink_message_t* msg)
{
    printf("on message id:%s, type:%d, code:%d", msg->msgid, msg->type, msg->code);
    switch (msg->type) {
        case THING_TYPE_MODEL_RSP:
            printf("Model data:%s", msg->data_string);
            break;

        case THING_TYPE_PROPERTY_SET:
            printf("property set:%s", msg->data_string);
            break;

        case THING_TYPE_PROPERTY_REPORT:
            printf("property report:%s", msg->data_string);
            break;

        case THING_TYPE_PROPERTY_REPORT_RSP:
            printf("property report reponse:%s", msg->data_string);
            break;

        default:
            break;
    }
    printf("\r\n");
}

int tuya_init(tuya_mqtt_context_t* context, char **args) {
    // log_set_quiet(true);
    int ret = OPRT_OK;

    ret = tuya_mqtt_init(context, &(const tuya_mqtt_config_t){
                             .host = "m1.tuyacn.com",
                             .port = 8883,
                             .cacert = tuya_cacert_pem,
                             .cacert_len = sizeof(tuya_cacert_pem),
                             .device_id = args[1],
                             .device_secret = args[2],
                             .keepalive = 100,
                             .timeout_ms = 2000,
                             .on_connected = NULL,
                             .on_disconnect = NULL,
                             .on_messages = on_messages
                         });
    if (ret != OPRT_OK) {
        return ret;
    }

    ret = tuya_mqtt_connect(context);

    return ret;
}

char* create_sysinfo_json() {
    struct sysinfo info;
    if (sysinfo(&info) == -1) {
        goto end;
    }

    char field_buffer[256];
    cJSON* packet = cJSON_CreateObject();

    snprintf(field_buffer, sizeof(field_buffer), "%ld", info.uptime);
    if (cJSON_AddStringToObject(packet, "uptime", field_buffer) == NULL) goto end;
    snprintf(field_buffer, sizeof(field_buffer), "%ld", info.totalram);
    if (cJSON_AddStringToObject(packet, "totalram", field_buffer) == NULL) goto end;
    snprintf(field_buffer, sizeof(field_buffer), "%ld", info.freeram);
    if (cJSON_AddStringToObject(packet, "freeram", field_buffer) == NULL) goto end;

    cJSON* loads = cJSON_AddArrayToObject(packet, "loads");
    if (loads == NULL) goto end;
    for (int i = 0; i < 3; i++) {
        snprintf(field_buffer, sizeof(field_buffer), "%ld", info.loads[i]);
        cJSON* load = cJSON_CreateString(field_buffer);
        if (load == NULL) goto end;
        cJSON_AddItemToArray(loads, load);
    }

    end:
    char* json_string = cJSON_Print(packet);
    cJSON_Delete(packet);
    cJSON_Minify(json_string);
    return json_string;
}
