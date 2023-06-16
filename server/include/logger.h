#ifndef __logger_h_
#define __logger_h_
#include <stdio.h>
#include <stdlib.h>

/*
*  Macros y funciones simples para log de errores.
*  EL log se hace en forma simple
*  Alternativa: usar syslog para un log mas completo. Ver secciÃ³n 13.4 del libro de  Stevens
*/

typedef enum {LOGGER_DEBUG=0, LOGGER_INFO, LOGGER_ERROR, LOGGER_FATAL} LOG_LEVEL;

extern LOG_LEVEL current_level;

/**
*  Minimo nivel de log a registrar. Cualquier llamada a log con un nivel mayor a newLevel sera ignorada
**/
void logger_set_log_lvl(LOG_LEVEL newLevel);

char * logger_get_lvl_description(LOG_LEVEL level);

// Debe ser una macro para poder obtener nombre y linea de archivo.
#define log(level, fmt, ...)   {if(level >= current_level) {\
	fprintf (stderr, "%s: %s:%d, ", logger_get_lvl_description(level), __FILE__, __LINE__); \
	fprintf(stderr, fmt, ##__VA_ARGS__); \
	fprintf(stderr,"\n"); }\
	if ( level==LOGGER_FATAL) exit(1);}
#endif
