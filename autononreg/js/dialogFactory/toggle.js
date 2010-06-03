    var toggle = new DFToggle("Toggle:");
    var dlgWizard = new DialogFactory("Dialog Factory toggle");

    
    toggle.value=0;
    dlgWizard.addControl(toggle);

    if (dlgWizard.show())
    {
                var out="toggle "+toggle.value;
                print(out);
    }
