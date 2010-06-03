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
print("res")
print(res)
print("source index:")
print(mnuSourceRatio.index)
print("dest index:")
print(mnuResolution.index)
