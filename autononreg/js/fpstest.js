//AD  <- These first 4 characters need to be the first 4 characters to identify the ECMAScript file to Avidemux
var app = new Avidemux();
var file="/work/samples/2mn.avi";
var goodfps=23976;
var fps;
/* Load file
*/
app.load(file);
/* Test get fps 
*/
fps=app.video.getFps1000();
//app.displayInfo("FPS "+fps);
if(fps==goodfps)
{
}
else
{
	app.displayError("Wrong fps ");
}
fps=10000;
app.video.setFps1000(fps);
fps=90;
fps=app.video.getFps1000();
//app.displayInfo("FPS "+fps);
if(fps!=10000)
{
	app.displayError("Testfps failed");
}

/* End of test
*/
