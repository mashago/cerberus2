
extern "C"
{
#include <stdio.h>
}
#include "cerberus.h"

int main(int argc, char** argv)
{
	printf("hello cerberus2\n");

	Cerberus c;
	c.start();

	return 0;
}
