
extern "C"
{
#include <stdio.h>
}
#include "cerberus_core.h"

int main(int argc, char** argv)
{
	printf("hello cerberus2\n");

	Cerberus c;
	c.start();

	return 0;
}
