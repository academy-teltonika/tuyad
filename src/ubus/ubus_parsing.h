#pragma once
#include <libubus.h>

void ubus_parse_system_info(struct ubus_request *req, int type, struct blob_attr *msg);

void ubus_parse_commesp_devices(struct ubus_request *req, int type, struct blob_attr *msg);

void ubus_parse_commesp_action_response(struct ubus_request *req, int type, struct blob_attr *msg);
