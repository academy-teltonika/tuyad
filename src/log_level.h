#pragma once
// This wrapper exists, because the Tuya logging module has some macro definitions that conflict with syslog.

enum LogLevel {
    LOG_LEVEL_EMERGENCY,
    LOG_LEVEL_ALERT,
    LOG_LEVEL_CRITICAL,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_NOTICE,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG
};