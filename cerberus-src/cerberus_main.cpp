
#include "cerberus.h"
#include "cerberus_log.h"

int main(int argc, char** argv)
{
	Log::info("hello cerberus2");

	Cerberus c;
	c.start();

	return 0;
}
