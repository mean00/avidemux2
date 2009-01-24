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
	displayInfo("Frame size 0 :"+app.video.frameSize(0)+" bytes");
	displayInfo("Frame size 1 :"+app.video.frameSize(1)+" bytes");

/* End of test
*/
