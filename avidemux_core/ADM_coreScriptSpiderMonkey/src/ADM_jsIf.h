extern "C"
{
	void jsHelp(JSContext *cx, const char *s);
	void jsPrint(JSContext *cx, const char *s);
	void jsPopupError(const char *s);
	void jsPopupInfo(const char *s);
}