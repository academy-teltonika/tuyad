#include "ubus.h"
#include "ubus_parsing.h"
#include "ubus_action_system.h"

const char* UbusSystemActionResult_messages[] = {
  [UBUS_SYSTEM_ACTION_RESULT_OK] = "Success.",
  [UBUS_SYSTEM_ACTION_RESULT_ERR_SYSTEM_NOT_FOUND] = "Failed to find \\\"system\\\" ubus server.",
  [UBUS_SYSTEM_ACTION_RESULT_ERR_ACTION_FAILED] = "Failed to invoke ubus method on \\\"system\\\".",
  [UBUS_SYSTEM_ACTION_RESULT_ERR_PARSE_FAILED] = "Failed to parse ubus response from \\\"system\\\"."
};

enum UbusSystemActionResult ubus_invoke_get_system_info(struct SystemInfo *systemInfo) {
  unsigned int id;
  if (ubus_lookup_id(g_ubus_context, "system", &id) != 0) {
    return UBUS_SYSTEM_ACTION_RESULT_ERR_SYSTEM_NOT_FOUND;
  } 
  if (ubus_invoke(g_ubus_context, id, "info", NULL, ubus_parse_system_info, systemInfo, 3000) != 0) {
    return UBUS_SYSTEM_ACTION_RESULT_ERR_ACTION_FAILED;
  }

  if (!systemInfo->parsed_successfuly) {
    return UBUS_SYSTEM_ACTION_RESULT_ERR_PARSE_FAILED;
  }

  return UBUS_SYSTEM_ACTION_RESULT_OK;
}
