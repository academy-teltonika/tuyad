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
  UBUS_SYSTEM_ACTION_RESULT_ERR_ACTION_FAILED,
  UBUS_SYSTEM_ACTION_RESULT_ERR_PARSE_FAILED,
};

extern const char* UbusSystemActionResult_messages[];

enum UbusSystemActionResult ubus_invoke_get_system_info(struct SystemInfo *systemInfo);
