#pragma once
#include "tuya_action_esp.h"
#include <libubus.h>
#include <stdbool.h>

extern struct ubus_context *g_ubus_context;

enum UbusCommespActionResult {
  UBUS_COMMESP_ACTION_RESULT_OK,
  UBUS_COMMESP_ACTION_RESULT_ERR_COMMESPD_NOT_FOUND,
  UBUS_COMMESP_ACTION_RESULT_ERR_COMMESPD_ACTION_FAILED,
};

const char* UbusActionResult_messages[] = {
  [UBUS_COMMESP_ACTION_RESULT_OK] = "Success.",
  [UBUS_COMMESP_ACTION_RESULT_ERR_COMMESPD_NOT_FOUND] = "Communication with ubus \"commespd\" module failed.",
  [UBUS_COMMESP_ACTION_RESULT_ERR_COMMESPD_ACTION_FAILED] = "Unknown failure while executing commespd action."
};

enum UbusSystemActionResult {
  UBUS_SYSTEM_ACTION_RESULT_OK,
  UBUS_SYSTEM_ACTION_RESULT_ERR_SYSTEM_NOT_FOUND,
};

const char* UbusSystemActinoResult_messages[] = {
  [UBUS_SYSTEM_ACTION_RESULT_OK] = "Success.",
  [UBUS_SYSTEM_ACTION_RESULT_ERR_SYSTEM_NOT_FOUND] = "Communication with ubus \"system\" module failed.",
};

enum UbusCommespActionResult ubus_invoke_esp_toggle_pin(struct EspRequest *request,
                                struct EspResponse *response);

enum UbusCommespActionResult ubus_invoke_list_esp_devices(struct EspDevices *devices);

enum UbusCommespActionResult ubus_invoke_esp_read_sensor(struct EspRequest *request,
                                struct EspResponse *response);

void create_ubus_message_from_esp_request(struct blob_buf *request_buf,
                                          struct EspRequest *request);

enum UbusSystemActionResult ubus_invoke_get_system_info(struct SystemInfo *systemInfo);

bool ubus_init();

void ubus_deinit();
