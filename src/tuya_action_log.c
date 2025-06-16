#include "tuya_action_log.h"
#include "tuya.h"

#include "log_level.h"
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
    fwrite("\n", sizeof(char), 1, file); // TODO: maybe should check for error

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
