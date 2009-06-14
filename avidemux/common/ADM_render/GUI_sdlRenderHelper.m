#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>

static NSWindow* nsParentWindow = nil;
static NSWindow* nsWindow = nil;
static NSQuickDrawView* nsView = nil;
static bool cocoaApplication = false;
static id oldDelegate = nil;
static int widgetX = 0, widgetY = 0;

@interface SdlCocoaWindow : NSWindow
- (void)windowDidMove:(id)notification;
- (void)windowWillMiniaturize:(NSNotification *)notification;
- (void)windowDidDeminiaturize:(NSNotification *)notification;
@end

@implementation SdlCocoaWindow
- (void)windowDidMove:(id)notification
{
	NSRect parentFrame = [nsParentWindow frame];
	NSPoint point = NSMakePoint(parentFrame.origin.x + widgetX, parentFrame.origin.y + widgetY);

	[self setFrameOrigin:point];
}

- (void)windowWillMiniaturize:(NSNotification *)notification
{
	[self setIsVisible:NO];
}

- (void)windowDidDeminiaturize:(NSNotification *)notification
{
	[self setIsVisible:YES];
}
@end

void initSdlCocoaView(void* parent, int x, int y, int width, int height, bool carbonParent)
{
	printf("[SDL] Creating Cocoa view x: %d, y: %d, width: %d, height %d, carbon %d\n", x, y, width, height, carbonParent);

	NSRect contentRect;

	if (carbonParent)
	{
		contentRect = NSMakeRect(x, y, width, height);

		if (!nsWindow)
		{
			// initWithWindowRef always returns the same result for the same WindowRef
			nsWindow = [[NSWindow alloc] initWithWindowRef:(WindowRef)parent];

			[nsWindow setContentView:[[[NSView alloc] initWithFrame:contentRect] autorelease]];
		}
	}
	else
	{
		contentRect = NSMakeRect(0, 0, width, height);
		
		if (!nsWindow)
		{
			nsWindow = [[SdlCocoaWindow alloc] initWithContentRect:contentRect styleMask:NSDocModalWindowMask backing:NSBackingStoreBuffered defer:NO];

			nsParentWindow = (NSWindow*)parent;
			widgetX = x;
			widgetY = y;
			oldDelegate = [nsParentWindow delegate];
			cocoaApplication = true;

			[nsWindow setLevel:NSFloatingWindowLevel];
			[nsWindow setHasShadow:NO];
			
			NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
	
			[nc addObserver:nsWindow selector:@selector(windowDidMove:) name:NSWindowDidMoveNotification object:nsParentWindow];
			[nc addObserver:nsWindow selector:@selector(windowWillMiniaturize:) name:NSWindowWillMiniaturizeNotification object:nsParentWindow];
			[nc addObserver:nsWindow selector:@selector(windowDidDeminiaturize:) name:NSWindowDidDeminiaturizeNotification object:nsParentWindow];
		}
	}

	if (!nsView)
	{
		nsView = [[NSQuickDrawView alloc] initWithFrame:contentRect];

		[[nsWindow contentView] addSubview:nsView];
		[nsView release];
		
		if (carbonParent)
			[nsWindow orderOut:nil];	// very important, otherwise window won't be placed correctly on repeat showings
		else
		{
			NSRect parentFrame = [nsParentWindow frame];
			NSPoint point = NSMakePoint(parentFrame.origin.x + x, parentFrame.origin.y + y);
			
			[nsWindow setFrameOrigin:point];
		}

		[nsWindow orderFront:nil];
	}
	else
	{
		[nsView setFrame:contentRect];
		[[nsWindow contentView] setFrame:contentRect];
	}

	// finally, set SDL environment variables with all this nonsense
	char SDL_windowhack[50];

	sprintf(SDL_windowhack,"SDL_NSWindowPointer=%ld",(int)nsWindow);
	putenv(SDL_windowhack);

	sprintf(SDL_windowhack,"SDL_NSQuickDrawViewPointer=%ld",(int)nsView);
	putenv(SDL_windowhack);
}

void destroyCocoaView(void)
{
	if (cocoaApplication)
	{
		[nsParentWindow setDelegate:oldDelegate];
		
		// we're using a floating panel so it doesn't automatically get destroyed
		[nsWindow release];
		
		nsWindow = nil;
		nsView = nil;
		oldDelegate = nil;
	}
	else if (nsWindow)
	{
		// Reference count cannot fall below 2 because SDL releases the window when closing
		// and again when reinitialising (even though this is our own window...).
		if ([nsWindow retainCount] > 2)
			[nsWindow release];

		// SDL takes care of all the destroying...a little too much, so make sure our Carbon
		// window is still displayed (via its Cocoa wrapper)
		[nsWindow makeKeyAndOrderFront:nil];
	}
}
