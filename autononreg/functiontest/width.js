//AD  <- These first 4 characters need to be the first 4 characters to identify the ECMAScript file to Avidemux
var app = new Avidemux();
var file="/work/samples/2mn.avi";
var goodwidth=512;
var goodheight=384;
var fps;
/* Load file
*/
print("Width Test Start");
app.load(file);
/* Test get fps 
*/
fps=app.video.getWidth();
//displayInfo("FPS "+fps);
if(fps==goodwidth)
{
}
else
{
	displayError("Wrong width "+fps+"expected "+goodwidth);
}
fps=app.video.getHeight();
//displayInfo("FPS "+fps);
if(fps==goodheight)
{
}
else
{
	displayError("Wrong height "+fps+"expected "+goodheight);
}
print("Width Test End");


/* End of test
*/
