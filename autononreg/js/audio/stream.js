//AD  <- These first 4 characters need to be the first 4 characters to identify the ECMAScript file to Avidemux
var app = new Avidemux();
var file="/work/samples/dual_avi.avi";
var fps;
/* Load file
*/
print("Audio Test Start");
app.load(file);
/* Test get fps 
*/
fps=app.audio.getNbTracks();
displayInfo("# "+fps);
app.audio.setTrack(1);
print("Audio Test End");

/* End of test
*/
