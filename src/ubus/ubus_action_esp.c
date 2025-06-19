#include "ubus_action_esp.h"
#include "ubus.h"
#include "ubus_parsing.h"
#include <libubus.h>

const char* UbusCommespActionResult_messages[] = {
  [UBUS_COMMESP_ACTION_RESULT_OK] = "Success.",
  [UBUS_COMMESP_ACTION_RESULT_ERR_COMMESPD_NOT_FOUND] = "Failed to find \\\"commespd\\\" ubus server.",
  [UBUS_COMMESP_ACTION_RESULT_ERR_COMMESPD_ACTION_FAILED] = "Failed to invoke ubus method on \\\"commespd\\\".",
  [UBUS_COMMESP_ACTION_RESULT_ERR_PARSE_FAILED] = "Failed to parse ubus response from \\\"commespd\\\"."
};

static void create_ubus_message_from_esp_request(struct blob_buf *request_buf,
                                          struct EspRequest *request) {
  if (request->port != NULL) {
    blobmsg_add_string(request_buf, "port", request->port);
  }
  blobmsg_add_u32(request_buf, "pin", request->pin);

  switch (request->tag) {
  case ESP_ACTION_READ_SENSOR:
    if (request->sensor != NULL) {
      blobmsg_add_string(request_buf, "sensor", request->sensor);
    }
    if (request->model != NULL) {
      blobmsg_add_string(request_buf, "model", request->model);
    }
    break;
  case ESP_ACTION_TOGGLE_PIN:
    break;
  }
}

enum UbusCommespActionResult ubus_invoke_esp_toggle_pin(struct EspRequest* request,
                                struct EspResponse *response) {
  unsigned int id;
  if (ubus_lookup_id(g_ubus_context, ESP_COMMUNICATION_DAEMON_NAME, &id) != 0) {
    return UBUS_COMMESP_ACTION_RESULT_ERR_COMMESPD_NOT_FOUND;
  }

  enum UbusCommespActionResult result = UBUS_COMMESP_ACTION_RESULT_OK;
  struct blob_buf ubus_message = {};
  blob_buf_init(&ubus_message, 0);
  create_ubus_message_from_esp_request(&ubus_message, request);
  if (ubus_invoke(g_ubus_context, id, request->pin_power ? "on" : "off",
                  ubus_message.head, ubus_parse_commesp_action_response,
                  response, 3000) != 0) {
    result = UBUS_COMMESP_ACTION_RESULT_ERR_COMMESPD_ACTION_FAILED;
  }

  blob_buf_free(&ubus_message);
  return result;
}

enum UbusCommespActionResult ubus_invoke_list_esp_devices(struct EspDevices *devices) {
  unsigned int id;
  if (ubus_lookup_id(g_ubus_context, ESP_COMMUNICATION_DAEMON_NAME, &id) != 0) {
    return UBUS_COMMESP_ACTION_RESULT_ERR_COMMESPD_NOT_FOUND;
  }

  if (ubus_invoke(g_ubus_context, id, "devices", NULL,
                  ubus_parse_commesp_devices, devices, 3000) != 0) {
    return UBUS_COMMESP_ACTION_RESULT_ERR_COMMESPD_ACTION_FAILED;
  }

  return UBUS_COMMESP_ACTION_RESULT_OK;
}

enum UbusCommespActionResult ubus_invoke_esp_read_sensor(struct EspRequest *request,
                                 struct EspResponse *response) {
  unsigned int id;
  if (ubus_lookup_id(g_ubus_context, ESP_COMMUNICATION_DAEMON_NAME, &id) != 0) {
    return UBUS_COMMESP_ACTION_RESULT_ERR_COMMESPD_NOT_FOUND;
  }

  enum UbusCommespActionResult result = UBUS_COMMESP_ACTION_RESULT_OK;
  struct blob_buf ubus_message = {};
  blob_buf_init(&ubus_message, 0);
  create_ubus_message_from_esp_request(&ubus_message, request);
  if (ubus_invoke(g_ubus_context, id, "get", ubus_message.head,
                  ubus_parse_commesp_action_response, response,
                  3000) != 0) {
    result = UBUS_COMMESP_ACTION_RESULT_ERR_COMMESPD_ACTION_FAILED;
  }

  blob_buf_free(&ubus_message);
  return result;
}

