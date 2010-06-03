//AD  <- These first 4 characters need to be the first 4 characters to identify the ECMAScript file to Avidemux
var app = new Avidemux();
var file="/work/samples/2mn.avi";
var goodfps=23976;
var fps;
/* Load file
*/
print("SaveJpeg Test Start");
app.load(file);
/* Test get fps 
*/
app.video.saveJpeg("../out/saveJ0.avi");
app.goToTime(0,1,0);
app.video.saveJpeg("../out/saveJ1.avi");
print("SaveJpeg Test End");

/* End of test
*/
