
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include "cerberus_util.h"

#ifndef WIN32

void *dl_load_lib(const char *path)
{
    void *lib = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
	return lib;
}

void *dl_load_func(void *lib, const char *sym)
{
    return (void *)dlsym(lib, sym);
}

void dl_unload_lib(void *lib)
{
	dlclose(lib);
}

char *dl_error(char *buffer, int size)
{
	strncpy(buffer, dlerror(), size-1);
	buffer[size-1] = '\0';
	return buffer;
}

#else

void *dl_load_lib(const char *path)
{
	HMODULE lib = LoadLibraryExA(path, NULL, 0);
	return lib;
}

void *dl_load_func(void *lib, const char *sym)
{
    return (void *)GetProcAddress((HMODULE)lib, sym);
}

void dl_unload_lib(void *lib)
{
	FreeLibrary((HMODULE)lib);
}

char *dl_error(char *buffer, int size)
{
	int error = GetLastError();
	if (FormatMessageA(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, error, 0, buffer, size, NULL))
	{
		return buffer;
	}

	snprintf(buffer, size, "system error %d\n", error);
	return buffer;
}

#endif
