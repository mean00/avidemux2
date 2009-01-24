//AD  <- These first 4 characters need to be the first 4 characters to identify the ECMAScript file to Avidemux
var app = new Avidemux();
var file="/work/samples/2mn.avi";
var goodwidth=512;
var goodheight=384;
var fps;
/* Load file
*/
print("fileRead Test Start");
/* Test get fps 
*/
fps=fileReadSelect();
app.load(fps);
print("fileRead Test End");


/* End of test
*/
