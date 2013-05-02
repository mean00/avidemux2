# check get_folder_content function
# return the list of files with extention ext
# full path is returned !
# i.e. in the example below you will get a list
# /work/samples/avi/foo.avi
# /work/samples/avi/bar.avi
# ...
#
print("Get folder content")
root="/work/samples/avi/2mn.avi"
filename=basename(root);
print(str(root)+"=>"+str(filename))
if(not filename == "2mn.avi"):
  raise
