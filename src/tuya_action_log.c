#include "tuya_action_log.h"
#include "tuya.h"

#include "log_level.h"
#include <sys/sysinfo.h>
#include <syslog.h>
#include <cJSON.h>
#include <stdlib.h>
#include <string.h>

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

char *create_sysinfo_json(struct SystemInfo *systemInfo) { // TODO static
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

static char *parse_log_message_json(char *json_string) {
    cJSON *json = cJSON_Parse(json_string);
    cJSON *inputParams_json = cJSON_GetObjectItem(json, "inputParams");
    cJSON *message_json = cJSON_GetObjectItem(inputParams_json, "message");

    char *message = calloc(strlen(message_json->valuestring) + 1, sizeof(char));
    strcpy(message, message_json->valuestring);
    cJSON_Delete(json);

    return message;
}

static enum FileOperationResult append_message_to_file(char *filepath, char *message) {
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

static void syslog_file_operation(enum FileOperationResult result, char *user_data) {
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
