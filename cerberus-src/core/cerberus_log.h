#pragma once

class Log
{
public:
	enum Level
	{
		LEVEL_ERROR = 0,
		LEVEL_WARN  = 1,
		LEVEL_INFO  = 2,
		LEVEL_DEBUG = 3
	};

	static void set_level(Level level);
	static Level get_level();

	static void error(const char* fmt, ...);
	static void warn(const char* fmt, ...);
	static void info(const char* fmt, ...);
	static void debug(const char* fmt, ...);

private:
	static Level min_level;
	static void output(Level level, const char* fmt, va_list args);
};
