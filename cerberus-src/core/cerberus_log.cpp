#include <cstdarg>
#include <cstdio>

#include "cerberus_log.h"

Log::Level Log::min_level = Log::LEVEL_DEBUG;

void Log::set_level(Level level)
{
	min_level = level;
}

Log::Level Log::get_level()
{
	return min_level;
}

void Log::output(Level level, const char* fmt, va_list args)
{
	if (level > min_level)
		return;

	const char* prefix = "";
	switch (level)
	{
	case LEVEL_ERROR: prefix = "[ERROR] "; break;
	case LEVEL_WARN:  prefix = "[WARN] ";  break;
	case LEVEL_INFO:  prefix = "[INFO] ";  break;
	case LEVEL_DEBUG: prefix = "[DEBUG] "; break;
	}

	fprintf(stderr, "%s", prefix);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	fflush(stderr);
}

void Log::error(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	output(LEVEL_ERROR, fmt, args);
	va_end(args);
}

void Log::warn(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	output(LEVEL_WARN, fmt, args);
	va_end(args);
}

void Log::info(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	output(LEVEL_INFO, fmt, args);
	va_end(args);
}

void Log::debug(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	output(LEVEL_DEBUG, fmt, args);
	va_end(args);
}
