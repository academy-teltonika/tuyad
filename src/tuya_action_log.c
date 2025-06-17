#include "tuya_action_log.h"
#include "tuya.h"
#include "log_level.h"
#include "tuya_error.h"
#include <assert.h>
#include <syslog.h>
#include <cJSON.h>
#include <stdlib.h>
#include <string.h>

const char *FILE_OPERATION_RESULT_messages[] = {
    "Success.",
    "Failed to open file.",
    "Failed to write to file.",
    "Failed to close file."
};

static char *parse_log_message_json(cJSON *action_json) {
    cJSON *inputParams_json = cJSON_GetObjectItem(action_json, "inputParams");
    cJSON *message_json = cJSON_GetObjectItem(inputParams_json, "message");

    char *message = calloc(strlen(message_json->valuestring) + 1, sizeof(char));
    strcpy(message, message_json->valuestring);

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
    ret = fwrite("\n", sizeof(char), 1, file);
    if (ret != 1) {
        fclose(file);
        return FILE_OPERATION_RESULT_ERROR_WRITE;
    }

    ret = fclose(file);
    if (ret == EOF) {
        return FILE_OPERATION_RESULT_ERROR_CLOSE;
    }

    return FILE_OPERATION_RESULT_OK;
}

static char *create_file_error_json(enum FileOperationResult result) {
  switch (result) {
  case FILE_OPERATION_RESULT_OK:
  	assert(false); // You passed a OK as an error.
  case FILE_OPERATION_RESULT_ERROR_OPEN:
    return create_tuya_response_json(FILE_OPERATION_RESULT_messages[FILE_OPERATION_RESULT_ERROR_OPEN], false);
  case FILE_OPERATION_RESULT_ERROR_WRITE:
    return create_tuya_response_json(FILE_OPERATION_RESULT_messages[FILE_OPERATION_RESULT_ERROR_WRITE], false);
  case FILE_OPERATION_RESULT_ERROR_CLOSE:
    return create_tuya_response_json(FILE_OPERATION_RESULT_messages[FILE_OPERATION_RESULT_ERROR_CLOSE], false);
  }
  assert(false); // Never happens, but compiler complains without it.
}

void execute_log_to_file(cJSON *action_json, char **response) {
    char* message = parse_log_message_json(action_json);    
    // TODO: Error handling.
    enum FileOperationResult result = append_message_to_file("/tmp/tuya_messages", message);
    if (result != FILE_OPERATION_RESULT_OK) {
        *response = create_file_error_json(result);
    } else {
        *response = create_tuya_response_json("Message written.", true);
    }
    free(message);
}

// static void syslog_file_operation(enum FileOperationResult result, char *user_data) {
//     switch (result) {
//         case FILE_OPERATION_RESULT_OK:
//             syslog(LOG_LEVEL_INFO, "Succesfully written action message to file.");
//             break;
//         case FILE_OPERATION_RESULT_ERROR_CLOSE:
//             syslog(LOG_LEVEL_WARNING, "Failed to close file");
//             break;
//         case FILE_OPERATION_RESULT_ERROR_OPEN:
//         case FILE_OPERATION_RESULT_ERROR_WRITE:
//             syslog(LOG_LEVEL_ERROR, "Could not write action message to file. %s",
//                    FILE_OPERATION_RESULT_messages[result]);
//             break;
//     }
// }
