#pragma once
#include <stdbool.h>

struct SystemInfo {
	unsigned long long total;
	unsigned long long free;
	int uptime;
	int load[3];
	bool parsed_successfuly;
};

char *create_sysinfo_json(struct SystemInfo *systemInfo); // TODO static
