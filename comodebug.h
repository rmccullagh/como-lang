#define DEBUG_TYPE(o) do { \
	fprintf(stderr, "%s:%d: O_TYPE:%d\n", __func__, __LINE__, O_TYPE(o)); \
} while(0)

static void como_debug_ex(const char *f,
		const char *fn, int ln, const char* format, ...)
{
	fputc('\n', stderr);
	fprintf(stderr, "debug: %s:%s:%d: ", f, fn, ln);
	va_list args;
	va_start (args, format);
	vfprintf (stderr, format, args);
	va_end (args);
	fflush(stderr);	
}

static void como_error_noreturn_ex(const char *f,
		const char *fn, int ln, const char* format, ...)
{
	fputc('\n', stderr);
	fprintf(stderr, "%s:%s:%d: fatal: ", f, fn, ln);
	va_list args;
	va_start (args, format);
	vfprintf (stderr, format, args);
	va_end (args);
	fflush(stderr);	
	exit(1);
}

#define como_error_noreturn(format, ...) como_error_noreturn_ex(__FILE__, __func__, __LINE__, format, ##__VA_ARGS__)
#ifdef COMO_DEBUG
#define como_debug(format, ...) como_debug_ex(__FILE__, __func__, __LINE__, format, ##__VA_ARGS__)
#else
#define como_debug(format, ...)
#endif

