#pragma once

#include "tuyalink_core.h"
#include "tuya_error_code.h"


char *create_sysinfo_json();
int tuya_init(tuya_mqtt_context_t* context, char **args);
void log_sysinfo(char* json_string);