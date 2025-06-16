char *create_sysinfo_json(struct SystemInfo *systemInfo) { // TODO static
    char *json_string = NULL;
    char field_buffer[256];
    cJSON *packet = cJSON_CreateObject();

    if (systemInfo == NULL) {
        goto end;
    }

    snprintf(field_buffer, sizeof(field_buffer), "%u", systemInfo->uptime);
    if (cJSON_AddStringToObject(packet, "uptime", field_buffer) == NULL) goto end;
    snprintf(field_buffer, sizeof(field_buffer), "%llu", systemInfo->total);
    if (cJSON_AddStringToObject(packet, "totalram", field_buffer) == NULL) goto end;
    snprintf(field_buffer, sizeof(field_buffer), "%llu", systemInfo->free);
    if (cJSON_AddStringToObject(packet, "freeram", field_buffer) == NULL) goto end;

    cJSON *loads = cJSON_AddArrayToObject(packet, "loads");
    if (loads == NULL) goto end;
    for (int i = 0; i < 3; i++) {
        snprintf(field_buffer, sizeof(field_buffer), "%u", systemInfo->load[i]);
        cJSON *load = cJSON_CreateString(field_buffer);
        if (load == NULL) goto end;
        cJSON_AddItemToArray(loads, load);
    }

end:
    json_string = cJSON_Print(packet);
    cJSON_Delete(packet);
    cJSON_Minify(json_string);
    return json_string;
}

