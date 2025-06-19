#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tuya_error.h"

static const char* TUYA_RESPONSE_FORMAT_ERROR = "{\"result\":\"err\", \"message\":\"%s\"}";
static const char* TUYA_RESPONSE_FORMAT_OK = "{\"result\":\"ok\", \"message\":\"%s\"}";

char* create_tuya_response_json(const char* message, bool ok) {
  char* response_json;
  if (ok) {
      int len = strlen(message) + strlen(TUYA_RESPONSE_FORMAT_OK) + 1;
      response_json = calloc(len, sizeof(char));
      snprintf(response_json, len, TUYA_RESPONSE_FORMAT_OK, message);
  } else {
      int len = strlen(message) + strlen(TUYA_RESPONSE_FORMAT_ERROR) + 1;
      response_json = calloc(len, sizeof(char));
      snprintf(response_json, len, TUYA_RESPONSE_FORMAT_ERROR, message);
  }

  return response_json;
}
