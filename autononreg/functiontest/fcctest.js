//AD  <- These first 4 characters need to be the first 4 characters to identify the ECMAScript file to Avidemux
var app = new Avidemux();
var file="/work/samples/2mn.avi";
var goodfcc="DIV3";
var fps;
/* Load file
*/
print("FCC Test Begin");
app.load(file);
/* Test get fps 
*/
fps=app.video.getFCC();
//displayInfo("FPS "+fps);
if(fps==goodfcc)
{
}
else
{
	displayError("Good:"+goodfcc+"Bad:"+fps);
	displayError("Wrong fcc ");
}
print("FCC Test End");
/* End of test
*/
