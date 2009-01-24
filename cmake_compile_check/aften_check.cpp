#include <string.h>
#include <aften/aften.h>

int main(int argc, char **argv)
{
	const char* aftenVersion = aften_get_version();

	if (strcmp(aftenVersion, "0.07") == 0)
		return 7;
	else if (strcmp(aftenVersion, "0.0.8") == 0)
		return 8;

	return 0;
}
