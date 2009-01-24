//AD  <- These first 4 characters need to be the first 4 characters to identify the ECMAScript file to Avidemux
var app = new Avidemux();
var file="/work/samples/2mn.avi";
var goodfcc="DIV3";
var fps;
/* Load file
*/
app.load(file);
/* Test get fps 
*/
fps=app.video.getFCC();
//app.displayInfo("FPS "+fps);
if(fps==goodfcc)
{
}
else
{
	app.displayError("Good:"+goodfcc+"Bad:"+fps);
	app.displayError("Wrong fcc ");
}

/* End of test
*/
