toggle = DFToggle("Toggle:");
dlgWizard = DialogFactory("Dialog Factory toggle");


toggle.value=0;
dlgWizard.addControl(toggle);

res=dlgWizard.show()
print("Dialog output ");
print(res);
print("Toggle value");
print(toggle.value);
