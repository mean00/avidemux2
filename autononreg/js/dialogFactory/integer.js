    var toggle = new DFInteger("Integer (5-10)",5,10);
    var dlgWizard = new DialogFactory("Dialog Factory toggle");

    
    toggle.value=7;
    dlgWizard.addControl(toggle);

    if (dlgWizard.show())
    {
                var out="toggle "+toggle.value;
                print(out);
    }
