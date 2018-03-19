#include <stdlib.h>
#include <stdio.h>

void main()
{
	char *version = getenv("VERSION_CONTROL");
	if (version)
		printf("%s\n", version);
}
