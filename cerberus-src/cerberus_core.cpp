
extern "C"
{
#include <stdio.h>
#include <unistd.h>
}
#include <thread>
#include "cerberus_core.h"

int Cerberus::cerberus_create_monopoly_thread_service()
{
	// TODO create thread, create service, push into service list, push start event
	return 0;
}

int Cerberus::cerberus_create_share_thread_service()
{
	// TODO create service, push into service list, push start event
	return 0;
}

void share_thread_run(int id)
{
	printf("share_thread_run=%d\n", id);
}

void cerberus_start()
{
	// TODO init share work threads

	int thread_count = 3;

	std::thread tl[thread_count];
	for (int i = 0; i < thread_count; ++i)
	{
		printf("i=%d\n", i);
		tl[i] = std::thread(share_thread_run, i);
	}
	
	for (int i = 0; i < thread_count; ++i)
	{
		tl[i].join();
	}
}
