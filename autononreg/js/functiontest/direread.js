//AD  <- These first 4 characters need to be the first 4 characters to identify the ECMAScript file to Avidemux
var app = new Avidemux();
var count;
var file;
/* Load file
*/
print("DirRead Test Start");
count=allFilesFrom("/work/samples/");
print("FPS "+count);
for(i=0;i<count;i++)
{
	print(nextFile());
}
print("DirRead Test End");

/* End of test
*/
