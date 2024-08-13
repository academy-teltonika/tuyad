#pragma once

#include "tuyalink_core.h"
#include "tuya_error_code.h"
#include "ubus.h"
#include "arguments.h" // Should probably not have this module depend on this.

char *create_sysinfo_json(struct SystemInfo *systemInfo);

int tuya_init(struct arguments args);

void log_sysinfo(char *json_string);
