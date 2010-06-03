    var aspectRatios = [[1, 1], [4, 3], [16, 9]];
    var mnuResolution = new DFMenu("Resolution:");
    var mnuSourceRatio = new DFMenu("Source Aspect Ratio:");
    var mnuDestinationRatio = new DFMenu("Destination Aspect Ratio:");
    var dlgWizard = new DialogFactory("Dialog Factory menu");
    var i;

    for (i = 0; i < aspectRatios.length; i++)
    {
        mnuSourceRatio.addItem(aspectRatios[i][0].toString() + ":" + aspectRatios[i][1].toString());
        mnuDestinationRatio.addItem(aspectRatios[i][0].toString() + ":" + aspectRatios[i][1].toString());
    }
    

    dlgWizard.addControl(mnuSourceRatio);
    dlgWizard.addControl(mnuDestinationRatio);

    if (dlgWizard.show())
    {
                var out="source index"+mnuSourceRatio.index+", dest index="+mnuDestinationRatio.index;
                print(out);
    }
