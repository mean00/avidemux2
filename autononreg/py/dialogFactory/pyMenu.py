mnuResolution = DFMenu("Resolution:");
mnuSourceRatio = DFMenu("Source Aspect Ratio:");
mnuSourceRatio.addItem("1:1")
mnuSourceRatio.addItem("4:3")
mnuSourceRatio.addItem("16:9")
mnuResolution.addItem("320x240")
mnuResolution.addItem("640x400")

dlgWizard = DialogFactory("Dialog Factory menu");

dlgWizard.addControl(mnuSourceRatio);
dlgWizard.addControl(mnuResolution);

res=dlgWizard.show()
if res==1:
        print("res="+str(res)+" menuSource="+str(mnuSourceRatio.index)+" Resolution="+str(mnuResolution.index)+"\n")
else:
        print("cancelled\n");
