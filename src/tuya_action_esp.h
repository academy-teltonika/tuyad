#pragma once

#include "tuya.h"
#include <cJSON.h>
#include <ubus_action_esp.h>

void execute_commesp_esp_pin_action(enum EspAction action, cJSON *tuya_action_json, char **esp_action_response_json_string);

void execute_commesp_list_devices(char **commesp_resposne_json);

enum EspAction EspAction_from_TuyaAction(enum TuyaAction tuya_action);
