#include "logger.h"

int log_init(struct Logger logger, const char* log_filename) {
    if(!log_filename) {
        logger.fp = stdout;
        return 0;
    }

    logger.fp = fopen(log_filename, "a");
    if(!logger.fp) {
        return -1;
    }

    return 0;
}

void log_close(struct Logger logger) {
    if(logger.fp && logger.fp != stdout) {
        fclose(logger.fp);
        logger.fp = NULL;
    }
}

void log_msg(struct Logger logger, const char* msg) {
    if(!logger.fp) {
        fprintf(stderr, "Logger not found, cannot log message.\n");
        return;
    }

    fprintf(logger.fp, "%s\n", msg);
    fflush(logger.fp);
}