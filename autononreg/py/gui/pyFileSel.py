gui=Gui()
red=gui.fileReadSelect("Select a file read")
mix=":::::::::: output = @"+red+"@\n"
mix
print(mix)
red=gui.fileWriteSelect("Select a file write")
print(">>>>>>>>>>>>>>>>>>>>>>output<"+str(red)+"@.\n")
