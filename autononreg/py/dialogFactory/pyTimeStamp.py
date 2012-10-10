teger = DFTimeStamp("TimeStamp (1:2:3,4):",0,30000);
dlgWizard = DialogFactory("Dialog Factory : TimeStamp");


teger.value=1*3600*1000+2*60*1000+3*1000+4;
dlgWizard.addControl(teger);
print("Dialog input ");
print(teger.value);
res=dlgWizard.show()
print("Dialog output ");
print(res);
print("Toggle value");
print(teger.value);
