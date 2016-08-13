#ifndef COMO_DEBUG_H_INCLUDED
#define COMO_DEBUG_H_INCLUDED

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

static void como_debug_ex(const char *f,
		const char *fn, int ln, const char* format, ...)
{
	fprintf(stderr, ANSI_COLOR_GREEN "debug: %s:%s:%d: ", f, fn, ln);
	va_list args;
	va_start (args, format);
	vfprintf (stderr, format, args);
	va_end (args);
	fprintf(stderr, ANSI_COLOR_RESET "\n");
	fflush(stderr);	
}

static void __attribute__ ((noreturn)) como_error_noreturn_ex(const char *f,
		const char *fn, int ln, const char* format, ...)
{
	fprintf(stderr, ANSI_COLOR_RED  "fatal: %s:%s:%d: ", f, fn, ln);
	va_list args;
	va_start (args, format);
	vfprintf (stderr, format, args);
	va_end (args);
	fprintf(stderr, ANSI_COLOR_RESET  "\n");
	#ifdef COMO_COMPILER
	fprintf(stderr, "Traceback (most recent call first):\n");
	como_print_stack_trace();
	#endif
	fflush(stderr);	
	exit(1);
}

#define como_error_noreturn(format, ...) como_error_noreturn_ex(__FILE__, __func__, __LINE__, format, ##__VA_ARGS__)

#define como_debug(format, ...) como_debug_ex(__FILE__, __func__, __LINE__, format, ##__VA_ARGS__)

#define COMO_OOM() do { \
	como_error_noreturn("out of memory"); \
} while(0)

#endif

