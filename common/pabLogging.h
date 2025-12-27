#pragma once
#include <stdio.h>

// Log Levels
typedef enum {
    PAB_LOGLEVEL__INFO,
    PAB_LOGLEVEL__WARNING,
    PAB_LOGLEVEL__ERROR
} LogLevel;

// Function Declaration
void logger_log(LogLevel level, const char* fmt, ...);

// Macros for convenience (Variadic macros C99+)
// Note: You must now use printf style formatting (%d, %s) instead of C++ style ({})
#define PAB_INFO(...) logger_log(PAB_LOGLEVEL__INFO, __VA_ARGS__)
#define PAB_WARN(...) logger_log(PAB_LOGLEVEL__WARNING, __VA_ARGS__)
#define PAB_ERR(...)  logger_log(PAB_LOGLEVEL__ERROR, __VA_ARGS__)
