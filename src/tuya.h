#pragma once

#include "arguments.h" // Should probably not have this module depend on this.

enum TuyaAction {
    TUYA_ACTION_ESP_LIST_DEVICES,
    TUYA_ACTION_ESP_TOGGLE_PIN,
    TUYA_ACTION_ESP_READ_SENSOR,
    TUYA_ACTION_LOG,
    TUYA_ACTION_SYSTEM_INFO,
};

int
tuya_init(struct arguments args);
