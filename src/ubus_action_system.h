#pragma once
#include <stdbool.h>

struct SystemInfo {
	unsigned long long total;
	unsigned long long free;
	int uptime;
	int load[3];
	bool parsed_successfuly;
};

enum UbusSystemActionResult {
  UBUS_SYSTEM_ACTION_RESULT_OK,
  UBUS_SYSTEM_ACTION_RESULT_ERR_SYSTEM_NOT_FOUND,
};

const char* UbusSystemActinoResult_messages[] = {
  [UBUS_SYSTEM_ACTION_RESULT_OK] = "Success.",
  [UBUS_SYSTEM_ACTION_RESULT_ERR_SYSTEM_NOT_FOUND] = "Communication with ubus \"system\" module failed.",
};

enum UbusSystemActionResult ubus_invoke_get_system_info(struct SystemInfo *systemInfo);
