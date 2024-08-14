#pragma once
#include "tuya_action.h"
#include "esp.h"
#include <cJSON.h>

void execute_esp_action(enum EspAction action, cJSON *action_json, char **esp_action_response_json_string);
enum EspAction EspAction_from_TuyaAction(enum TuyaAction tuya_action);
