//AD  <- 
/*
	Simple script that scans the orgDir directory
	and unpack all avi files
	The resuling file is put in destDir directory

	Using new directorySearch API

*/
var app = new Avidemux();
var orgDir;
var destDir;
var reg =new RegExp(".$");
/*
	This is for unix
	For windows change to
	sep="\\";
	reg2=new RegExp("\\.*\\");
*/
var sep="/";
var extension;
var target;
//
//
//
// 	select files from original & target directories
//
	orgDir=fileWriteSelect();
	displayInfo("Selected :"+orgDir);

