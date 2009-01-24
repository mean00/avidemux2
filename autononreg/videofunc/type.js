//AD  <- These first 4 characters need to be the first 4 characters to identify the ECMAScript file to Avidemux
var app = new Avidemux();
var file="/work/samples/2mn.avi";
var nbf;
/* Load file
*/
app.load(file);
/* Test get nbf 
*/
nbf=app.video.getNbFrames();
	displayInfo("Frame type 0 :"+app.video.frameType(0)+" [16:I,0:P,BIG=B]");
	displayInfo("Frame type 1 :"+app.video.frameType(1));

/* End of test
*/
