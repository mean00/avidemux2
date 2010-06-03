//AD  <- These first 4 characters need to be the first 4 characters to identify the ECMAScript file to Avidemux
var app = new Avidemux();
var file="/work/samples/dual_avi.avi";
var fps;
/* Load file
*/
print("lamepreset Test Start");
app.load(file);
app.audio.codec("lame",128);
//displayInfo("Changing preset");
/* Test get fps 
*/
if(!app.audio.lamePreset("CBR"))
{
	displayError("CBR preset failed");
}
if(!app.audio.lamePreset("ABR"))
{
	displayError("ABR preset failed");
}
if(!app.audio.lamePreset("EXtreme"))
{
	displayError("Extreme preset failed");
}
print("lamepreset Test End");

/* End of test
*/
