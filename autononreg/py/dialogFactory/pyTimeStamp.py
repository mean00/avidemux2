teger = DFTimeStamp("Integer(10/20):",10,20);
dlgWizard = DialogFactory("Dialog Factory : TimeStamp");


teger.value=15;
dlgWizard.addControl(teger);

res=dlgWizard.show()
print("Dialog output ");
print(res);
print("Toggle value");
print(teger.value);
