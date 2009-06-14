#import <Cocoa/Cocoa.h>

int getMainNSWindow(void)
{
	NSEnumerator *enumerator = [[NSApp windows] objectEnumerator];
	id nsWindow;
	id mainWindow = 0;

	while ((nsWindow = [enumerator nextObject]))
	{
		if ([[nsWindow title] hasSuffix:@"Avidemux"])
		{
			mainWindow = nsWindow;
			break;
		}
	}

	return (int)mainWindow;
}