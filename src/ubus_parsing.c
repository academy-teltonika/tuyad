#include "ubus_parsing.h"
#include "ubus_action_esp.h"
#include "ubus_action_system.h"

#include <assert.h>

enum {
	SYSTEM_INFO_MEMORY,
	SYSTEM_INFO_LOAD,
	SYSTEM_INFO_UPTIME,
	__SYSTEM_INFO_MAX,
};

enum {
	SYSTEM_INFO_MEMORY_TOTAL,
	SYSTEM_INFO_MEMORY_FREE,
	__SYSTEM_INFO_MEMORY_MAX,
};

enum {
	ESP_RESPONSE_RESULT,
	ESP_RESPONSE_MESSAGE,
	ESP_RESPONSE_DATA,
	__ESP_RESPONSE_MAX
};

enum {
	ESP_RESPONSE_SENSOR_TEMPERATURE,
	ESP_RESPONSE_SENSOR_HUMIDITY,
	__ESP_RESPONSE_SENSOR_MAX
};

enum {
	COMMESP_DEVICES,
	__COMMESP_MAX
};

enum {
	COMMESP_DEVICE_PORT,
	COMMESP_DEVICE_VID,
	COMMESP_DEVICE_PID,
	__COMMESP_DEVICE_MAX
};

static const struct blobmsg_policy system_info_policy[__SYSTEM_INFO_MAX] = {
	[SYSTEM_INFO_MEMORY] = {.name = "memory", .type = BLOBMSG_TYPE_TABLE},
	[SYSTEM_INFO_UPTIME] = {.name = "uptime", .type = BLOBMSG_TYPE_INT32},
	[SYSTEM_INFO_LOAD] = {.name = "load", .type = BLOBMSG_TYPE_ARRAY},
};

static const struct blobmsg_policy system_info_memory_policy[__SYSTEM_INFO_MEMORY_MAX] = {
	[SYSTEM_INFO_MEMORY_TOTAL] = {.name = "total", .type = BLOBMSG_TYPE_INT64},
	[SYSTEM_INFO_MEMORY_FREE] = {.name = "free", .type = BLOBMSG_TYPE_INT64},
};

static const struct blobmsg_policy esp_action_response_policy[__ESP_RESPONSE_MAX] = {
	[ESP_RESPONSE_RESULT] = {.name = "result", .type = BLOBMSG_TYPE_STRING},
	[ESP_RESPONSE_MESSAGE] = {.name = "message", .type = BLOBMSG_TYPE_STRING},
	[ESP_RESPONSE_DATA] = {.name = "data", .type = BLOBMSG_TYPE_TABLE},
};

static const struct blobmsg_policy esp_action_response_sensor_data_policy[__ESP_RESPONSE_SENSOR_MAX] = {
	[ESP_RESPONSE_SENSOR_TEMPERATURE] = {.name = "temperature", BLOBMSG_TYPE_DOUBLE},
	[ESP_RESPONSE_SENSOR_HUMIDITY] = {.name = "humidity", BLOBMSG_TYPE_DOUBLE},
};

static const struct blobmsg_policy commesp_policy[__COMMESP_MAX] = {
	[COMMESP_DEVICES] = {.name = "devices", .type = BLOBMSG_TYPE_ARRAY},
};

static const struct blobmsg_policy commesp_device_properties_policy[__COMMESP_DEVICE_MAX] = {
	[COMMESP_DEVICE_PORT] = {.name = "port", .type = BLOBMSG_TYPE_STRING},
	[COMMESP_DEVICE_VID] = {.name = "vid", .type = BLOBMSG_TYPE_STRING},
	[COMMESP_DEVICE_PID] = {.name = "pid", .type = BLOBMSG_TYPE_STRING},
};

void ubus_parse_commesp_esp_action_response(struct ubus_request *req, int type, struct blob_attr *msg) {
	struct EspResponse *esp_response = (struct EspResponse *) req->priv;
	struct blob_attr *esp_response_table[__ESP_RESPONSE_MAX];

	blobmsg_parse(esp_action_response_policy, __ESP_RESPONSE_MAX, esp_response_table, blob_data(msg), blob_len(msg));

	if (esp_response_table[ESP_RESPONSE_RESULT] == NULL) {
		esp_response->parsed_successfuly = false;
		return;
	}

	char *result = blobmsg_get_string(esp_response_table[ESP_RESPONSE_RESULT]);
	esp_response->success = strcmp(result, "ok") == 0 ? true : false;

	if (esp_response_table[ESP_RESPONSE_MESSAGE] != NULL) {
		char *message = blobmsg_get_string(esp_response_table[ESP_RESPONSE_MESSAGE]);
		esp_response->message = calloc(strlen(message) + 1, sizeof(char));
		strcpy(esp_response->message, message);
	}

	// TODO move out into function.
  struct blob_attr *dht_table[__ESP_RESPONSE_SENSOR_MAX];

  switch (esp_response->tag) {
  case ESP_ACTION_TOGGLE_PIN:
    // Toggle pin must not return any data - if it does - something's wrong.
    assert(esp_response_table[ESP_RESPONSE_DATA] == NULL);
    break;
  case ESP_ACTION_READ_SENSOR:
    if (esp_response_table[ESP_RESPONSE_DATA] == NULL) {
      // TODO: MAKE ESPCOMMD RETURN ERR ON "DHT RETURNED NO DATA" INSTEAD OF OK.
      // if (esp_response->success == true) {
      //   esp_response->parsed_successfuly = false;
      // }
      break;
    }
    blobmsg_parse(esp_action_response_sensor_data_policy,
                  __ESP_RESPONSE_SENSOR_MAX, dht_table,
                  blobmsg_data(esp_response_table[ESP_RESPONSE_DATA]),
                  blobmsg_data_len(esp_response_table[ESP_RESPONSE_DATA]));
    if (dht_table[ESP_RESPONSE_SENSOR_TEMPERATURE] != NULL &&
        dht_table[ESP_RESPONSE_SENSOR_HUMIDITY] != NULL) {
      esp_response->sensor_reading->temperature =
          blobmsg_get_double(dht_table[ESP_RESPONSE_SENSOR_TEMPERATURE]);
      esp_response->sensor_reading->humidity =
          blobmsg_get_double(dht_table[ESP_RESPONSE_SENSOR_HUMIDITY]);
    } else {
      esp_response->parsed_successfuly = false;
    }
    break;
  }
}

void ubus_parse_commesp_devices(struct ubus_request *req, int type, struct blob_attr *msg) {
	struct EspDevices *device_list = (struct EspDevices *) req->priv;

	struct blob_attr *tb[__COMMESP_MAX];
	blobmsg_parse(commesp_policy, __COMMESP_MAX, tb, blobmsg_data(msg), blobmsg_data_len(msg));
	if (tb[COMMESP_DEVICES] == NULL) {
		goto failure;
	}

	struct blob_attr *devices = tb[COMMESP_DEVICES];
	struct blob_attr *device;
	size_t rem;

	device_list->count = 0;
	blobmsg_for_each_attr(device, devices, rem) {
		device_list->count++;
	}
	if (device_list->count > 0) {
		device_list->devices = malloc(device_list->count * sizeof(struct EspDevice));
	}

	int i = 0;
	blobmsg_for_each_attr(device, devices, rem) {
		struct blob_attr *device_properties[__COMMESP_DEVICE_MAX];
		blobmsg_parse(commesp_device_properties_policy, __COMMESP_DEVICE_MAX, device_properties,
		              blobmsg_data(device), blobmsg_data_len(device));
		if (device_properties[COMMESP_DEVICE_PORT] == NULL ||
		    device_properties[COMMESP_DEVICE_VID] == NULL ||
		    device_properties[COMMESP_DEVICE_PID] == NULL) {
			goto failure;
		}

		char *port = blobmsg_get_string(device_properties[COMMESP_DEVICE_PORT]);
		device_list->devices[i].port = calloc(strlen(port) + 1, sizeof(char));
		strcpy(device_list->devices[i].port, port);
		char *vid = blobmsg_get_string(device_properties[COMMESP_DEVICE_VID]);
		device_list->devices[i].vid = calloc(strlen(vid) + 1, sizeof(char));
		strcpy(device_list->devices[i].vid, vid);
		char *pid = blobmsg_get_string(device_properties[COMMESP_DEVICE_PID]);
		device_list->devices[i].pid = calloc(strlen(pid) + 1, sizeof(char));
		strcpy(device_list->devices[i].pid, pid);
		i++;
	}

	return;

failure:
	// Indicates a parsing error, although it would probably better to handle it in a more clear way.
	device_list->count = -2;
	if (device_list->count > 0) {
		free(device_list->devices);
	}
}

void ubus_parse_system_info(struct ubus_request *req, int type, struct blob_attr *msg) {
	struct SystemInfo *systemInfo = (struct SystemInfo *) req->priv;

	struct blob_attr *system_info[__SYSTEM_INFO_MAX];
	blobmsg_parse(system_info_policy, __SYSTEM_INFO_MAX, system_info, blobmsg_data(msg), blobmsg_len(msg));
	if (system_info[SYSTEM_INFO_MEMORY] == NULL ||
	    system_info[SYSTEM_INFO_LOAD] == NULL ||
	    system_info[SYSTEM_INFO_UPTIME] == NULL) {
		systemInfo->parsed_successfuly = false;
		return;
	}

	struct blob_attr *system_info_memory[__SYSTEM_INFO_MEMORY_MAX];
	blobmsg_parse(system_info_memory_policy, __SYSTEM_INFO_MEMORY_MAX, system_info_memory,
	              blobmsg_data(system_info[SYSTEM_INFO_MEMORY]),
	              blobmsg_data_len(system_info[SYSTEM_INFO_MEMORY]));

	systemInfo->total = blobmsg_get_u64(system_info_memory[SYSTEM_INFO_MEMORY_TOTAL]);
	systemInfo->free = blobmsg_get_u64(system_info_memory[SYSTEM_INFO_MEMORY_FREE]);
	systemInfo->uptime = blobmsg_get_u32(system_info[SYSTEM_INFO_UPTIME]);
	systemInfo->parsed_successfuly = true;

	struct blob_attr *loads = system_info[SYSTEM_INFO_LOAD];
	struct blob_attr *cur;
	size_t rem;
	int i = 0;
	blobmsg_for_each_attr(cur, loads, rem) {
		if (i >= 3) {
			break;
		}
		systemInfo->load[i] = blobmsg_get_u32(cur);
		i++;
	}
}
