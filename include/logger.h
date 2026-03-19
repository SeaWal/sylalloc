#ifndef SYL_LOGGER_H
#define SYL_LOGGER_H

#include <stdio.h>

struct Logger {
    FILE* fp; 
};

int log_init(struct Logger* logger, const char* log_filename);
void log_close(struct Logger* logger);
void log_msg(struct Logger* logger, const char* msg, ...);

#define LOG(logger, msg) log_msg((logger), "%s:%d (%s): %s", __FILE__, __LINE__+1, __func__, (msg))

#endif