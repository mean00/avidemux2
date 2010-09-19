//AD  <- These first 4 characters need to be the first 4 characters to identify the ECMAScript file to Avidemux
var app = new Avidemux();
var file="/work/samples/avi/2mn.avi";
var dir="/tmp/";
/* Load file
*/
if(!app.load(file))
{
	displayError("Failed to load "+file);
}
/*
	Save using all codec with their default settings
*/
app.setContainer("AVI");
/*

*/
doit("x264","xvid4");
/* End of test
*/
function doit(codec,out)
{
app.video.codec("copy","CQ=4","");
app.save(dir+out+".avi");
return true;

}
