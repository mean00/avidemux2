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
	displayInfo("Nb frames:"+nbf);

/* End of test
*/
