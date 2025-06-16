enum UbusSystemActionResult ubus_invoke_get_system_info(struct SystemInfo *systemInfo) {
  unsigned int id;
  if (ubus_lookup_id(g_ubus_context, "system", &id) ||
      ubus_invoke(g_ubus_context, id, "info", NULL, ubus_parse_system_info, systemInfo,
                  3000)) {
    syslog(LOG_LEVEL_ERROR, "Failed to request system info from system.");
    return UBUS_SYSTEM_ACTION_RESULT_ERR_SYSTEM_NOT_FOUND;
  }

  if (!systemInfo->parsed_successfuly) {
    syslog(LOG_LEVEL_DEBUG, "Failed to parse system info from ubus.");
    return NULL;
  }

  return systemInfo;
}
