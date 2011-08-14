adm=Avidemux()
editor=Editor()
teger = DFInteger("Integer(0/2000):",0,2000);
dlgWizard = DialogFactory("Frame to dump ");


teger.value=0;
dlgWizard.addControl(teger);

res=dlgWizard.show()

editor.hexDumpFrame(teger.value)
print("Done . ")

