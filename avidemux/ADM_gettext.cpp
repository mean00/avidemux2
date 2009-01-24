#include "config.h"
#ifdef HAVE_GETTEXT
#include "ADM_default.h"

#include <stdio.h>
#include <libintl.h>
#include <locale.h>

void initGetText(void)
{
	char *local = setlocale(LC_ALL, "");

#if !defined(__APPLE__)
	char *localeDir = ADM_getInstallRelativePath("share", "locale");
#else
    char *localeDir = ADM_getInstallRelativePath("..", "Resources", "locale");
#endif
	bindtextdomain("avidemux", localeDir);
	delete [] localeDir;

	bind_textdomain_codeset("avidemux", "UTF-8");

	if(local)
		printf("\n[Locale] setlocale %s\n", local);

	local = textdomain(NULL);
	textdomain("avidemux");

	if(local)
		printf("[Locale] Textdomain was %s\n", local);

	local = textdomain(NULL);

	if(local)
		printf("[Locale] Textdomain is now %s\n", local);

	printf("[Locale] Test: %s\n\n", dgettext("avidemux", "_File"));
};
#endif
