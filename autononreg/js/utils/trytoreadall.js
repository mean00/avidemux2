//AD  <- These first 4 characters need to be the first 4 characters to identify the ECMAScript file to Avidemux
/************************/
var basedir="/work/samples/";

var extension="avi";
/************************/
var app = new Avidemux();
var count;
var file;
var reg;
/* Load file
*/
print("DirRead Test Start");
count=allFilesFrom(basedir);
reg=new RegExp("\."+extension+"$");
print("Nb "+count);
print("Match "+reg);
for(i=0;i<count;i++)
{
	file=nextFile();
	if(!file.match(reg))
	{
		print(file+":does not match");
		continue;
	}
	print(file+":match");
	//displayInfo(file);
	/* Process it */
	app.forceUnpack(); // automatic unpack
	if(!app.load(file))
	{
		displayInfo("Faild to load "+file);
		continue;
	}
}
print("DirRead Test End");

/* End of test
*/
