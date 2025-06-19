#pragma once

#include "cJSON.h"

enum FileOperationResult {
    FILE_OPERATION_RESULT_OK,
    FILE_OPERATION_RESULT_ERROR_OPEN,
    FILE_OPERATION_RESULT_ERROR_WRITE,
    FILE_OPERATION_RESULT_ERROR_CLOSE
};

extern const char *FILE_OPERATION_RESULT_messages[];

void execute_log_to_file(cJSON *action_json, char **response);
