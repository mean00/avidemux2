/**************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Installer Framework.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
**************************************************************************/

// Derived from dynamic page example

var ComponentSelectionPage = null;
var update=false;

function Controller()
{
 installer.setMessageBoxAutomaticAnswer("OverwriteTargetDirectory", QMessageBox.Yes);
}
var Dir = new function () {
    this.toNativeSparator = function (path) {
        if (systemInfo.productType === "windows")
            return path.replace(/\//g, '\\');
        return path;
    }
};

function Component() 
{
	console.log("xxxxxxxxxxxx");
    if (installer.isInstaller()) 
	{		
		update=false;
        component.loaded.connect(this, Component.prototype.installerLoaded);
        ComponentSelectionPage = gui.pageById(QInstaller.ComponentSelection);
		
		var folder=installer.value( "TargetDir");
		if (installer.fileExists(folder + "/components.xml")) 
		{							
			update=true;
    	    // remove old one
			// https://stackoverflow.com/questions/46455360/workaround-for-qt-installer-framework-not-overwriting-existing-installation
			QMessageBox.question("question","Uninstall","I will now uninstall the previous version of avidemux VC++" ,  QMessageBox.Yes);     			
		    if (installer.fileExists(folder + "/scripts/auto_uninstall.js")) 
			    installer.execute(folder+"/Uninstall Avidemux VC++ 64bits.exe", "--script="+folder+"/scripts/auto_uninstall.js");			
            else
			    installer.execute(folder+"/Uninstall Avidemux VC++ 64bits.exe");
		}        

        installer.setDefaultPageVisible(QInstaller.TargetDirectory, !update);
        installer.setDefaultPageVisible(QInstaller.ComponentSelection, !update);
        installer.setDefaultPageVisible(QInstaller.LicenseCheck, !update);
		/*
        if (systemInfo.productType === "windows")
            installer.setDefaultPageVisible(QInstaller.StartMenuSelection, !update);
			*/
        installer.setDefaultPageVisible(QInstaller.ReadyForInstallation, false);
    }
}
Component.prototype.installerLoaded = function () 
{
/*
	if(update==true)
	{	
		QMessageBox.question("quit.question","zzz" , text, QMessageBox.Yes);     
	}
	*/
/*
	 if (installer.addWizardPage(component, "TargetWidget", QInstaller.TargetDirectory)) 
	 {
        var widget = gui.pageWidgetByObjectName("DynamicTargetWidget");
        if (widget != null) {
            widget.targetChooser.clicked.connect(this, Component.prototype.chooseTarget);
            widget.targetDirectory.textChanged.connect(this, Component.prototype.targetChanged);

            widget.windowTitle = "Installation Folder";
            widget.targetDirectory.text = Dir.toNativeSparator(installer.value("TargetDir"));
        }
    }
	*/
}
Component.prototype.targetChanged = function (text) {
/*
    var widget = gui.pageWidgetByObjectName("DynamicTargetWidget");
	 
    if (widget != null && text != "") 
	{
	   if (installer.fileExists(text + "/components.xml")) 
		{				
			QMessageBox.question("quit.question","zzz" , text, QMessageBox.Yes);     
			installer.setValue("TargetDir", text);
			update=true;
			widget.complete = true;
		}    
    }
	if(update==false)
	{
		installer.setDefaultPageVisible(QInstaller.ComponentSelection, true);
        installer.setDefaultPageVisible(QInstaller.LicenseCheck, true);
        if (systemInfo.productType === "windows")
            installer.setDefaultPageVisible(QInstaller.StartMenuSelection, true);
	}else
	{
	  gui.clickButton(buttons.NextButton); 
	}
	*/
}

Component.prototype.chooseTarget = function () {
	QMessageBox.question("quit.question", "chooseTarget",  QMessageBox.Yes);
    var widget = gui.pageWidgetByObjectName("DynamicTargetWidget");
    if (widget != null) {
        var newTarget = QFileDialog.getExistingDirectory("Choose your target directory.", widget.targetDirectory.text);
        if (newTarget != "")
            widget.targetDirectory.text = Dir.toNativeSparator(newTarget);
    }
}


Component.prototype.createOperationsForArchive = function(archive)
{
    // don't use the default operation
    component.createOperationsForArchive(archive);
	component.addOperation("CreateShortcut",
                "@TargetDir@/avidemux.exe",
                "@StartMenuDir@/avidemux.lnk",
                "workingDirectory=@TargetDir@");
	component.addOperation("CreateShortcut",
                "@TargetDir@/avidemux_jobs.exe",
                "@StartMenuDir@/avidemux_jobs.lnk",
                "workingDirectory=@TargetDir@");	
	component.addOperation("CreateShortcut",
                "@TargetDir@/@MaintenanceToolName@.exe",
                "@StartMenuDir@/@MaintenanceToolName@.lnk",
                "workingDirectory=@TargetDir@");	
}
