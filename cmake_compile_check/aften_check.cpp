#include <string.h>
#include <stdio.h>
#include <aften/aften.h>

int main(int argc, char **argv)
{
	const char* aftenVersion = aften_get_version();
        fprintf(stderr,"Aften version = %s\n",aftenVersion);
	if (strcmp(aftenVersion, "0.07") == 0)
		return 7;
	else if (strcmp(aftenVersion, "0.0.8") == 0)
		return 8;
	else if (strcmp(aftenVersion, "SVN") == 0)
		return 99;

	return -1;
}
