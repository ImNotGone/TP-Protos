#include "logger.h"

LOG_LEVEL current_level = LOGGER_DEBUG;

void logger_set_log_lvl(LOG_LEVEL newLevel) {
	if ( newLevel >= LOGGER_DEBUG && newLevel <= LOGGER_FATAL ) {
	   current_level = newLevel;
    }
}

char * logger_get_lvl_description(LOG_LEVEL level) {
    static char * description[] = {"DEBUG", "INFO", "ERROR", "FATAL"};
    if (level < LOGGER_DEBUG || level > LOGGER_FATAL) {
        return "";
    }
    return description[level];
}
