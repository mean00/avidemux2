#
#
import shutil
import re
import subprocess
import os
home=os.getenv("HOME")
print "Preparing independant bundle, home="+home+"..."
rootFolder=home+"/Avidemux2.6.app/Contents/Resources"
libFolder=rootFolder+"/lib"
binFolder=rootFolder+"/bin"
frameWorkFolder=rootFolder+"/../Frameworks"
qtPluginFolder=rootFolder+"/../plugins"
qts = ['QtCore', 'QtGui', 'QtOpenGL','QtScript','QtWidgets','QtPrintSupport','QtNetwork', 'QtDBus']

#
#
allSymbols=[]
#

def log(s):
    #print "Log:<"+str(s)+">"
    pass
#
#
#
def getShortName(fl):
    shortName=re.sub('^.*\/','',fl)
    return shortName
#
# Returns the list of libs as dependencies in /opt (macport)
#
def getGlobalDeps(target):
    cmd = ["/usr/bin/otool","-L", target]
    log(cmd)
    q = []
    cmd = subprocess.Popen(" ".join(cmd), shell=True, stdout=subprocess.PIPE)
    for line in cmd.stdout:
       line = re.sub('[ \t\r\n]*', '', line)
       line = re.sub('\(.*$', '', line)
       if(not line.startswith('/opt/local') and not line.startswith('/usr/local/opt') and not line.startswith('/usr/local/Cellar/qt5/') and not line.startswith('@rpath')):
              continue
       if(":" in line):
              continue
       log(line)
       q.append(line)
    log(str(q))
    return q;
#
#
#
def getGlobalDepsNoQt(target):
    q= []
    p = getGlobalDeps(target)
    for line in p:
       if not getShortName(line).startswith('Qt'):
          q.append(line)
    return q
#
def getGlobalDepsQtOnly(target):
    q= []
    p = getGlobalDeps(target)
    for line in p:
       if getShortName(line).startswith('Qt'):
          q.append(line)
    return q
#
def getRpathDepsOnly(target):
    q= []
    p = getGlobalDeps(target)
    for line in p:
       if getShortName(line).startswith('@rpath'):
          q.append(line)
    return q
#
# Returns the list of libs as dependencies  to local installation folder (
#
def getLocalDeps(target):
    cmd = ["/usr/bin/otool","-L", target]
    log(cmd)
    q = []
    cmd = subprocess.Popen(" ".join(cmd), shell=True, stdout=subprocess.PIPE)
    for line in cmd.stdout:
       line = re.sub('[ \t\r\n]*', '', line)
       line = re.sub('\(.*$', '', line)
       if(not line.startswith(libFolder)):
              continue
       if(":" in line):
              continue
       log(line)
       q.append(line)
    log(str(q))
    return q;
#
# 
# Copy used library coming from /opt/local
#
def copyFiles(folder,libFolder):
    copied=0
    log("Copy files"+folder)
    for dirname, dirnames, filenames in os.walk(folder):
       for filename in filenames:
           absPath=os.path.join(dirname, filename)
           deps=getGlobalDepsNoQt(absPath)
           for dep in deps:
               shortName=re.sub('^.*\/','',dep)
               if(os.path.exists(libFolder+'/'+shortName)):
                               log(shortName+" already copied")
               else:
                   log("Copying"+shortName)
                   myCopy(dep,libFolder)
                   copied+=1
    return copied
#
# 
# Copy used library coming from /opt/local
#
def copyQtDeps(components,libFolder):
    copied=0
    for modul in components:
        absPath='/opt/local/libexec/qt5/lib/'+modul+'.framework/Versions/5/'+modul
        print("Copy deps for "+modul+" ("+absPath+")")
        deps=getGlobalDepsNoQt(absPath)
        for dep in deps:
                   shortName=re.sub('^.*\/','',dep)
                   if(os.path.exists(libFolder+'/'+shortName)):
                               print(shortName+" already copied")
                   else:
                       print("Copying:"+shortName)
                       myCopy(dep,libFolder)
                       copied+=1
    # copy plugins deps too
    copyFiles(qtPluginFolder+'/imageformats',libFolder)
    copyFiles(qtPluginFolder+'/platforms',libFolder)
    return copied
##
def changeSymbol(target,oldName,newName):
        cmd="/usr/bin/install_name_tool -change "+oldName+" "+newName+" "+target
        log(cmd)
        subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
#
def changeId(target,newId):
           cmd="/usr/bin/install_name_tool -id "+newId+" "+target
           log(cmd)
           subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
#
#
def renameSymbols(libs):
   for qt in libs:
      print "Processing "+qt
      # Change binding
      for otherQt in allSymbols:
          newname=re.sub(r'^.*\/',r'@executable_path/../lib/',otherQt)
          exc="install_name_tool -change "+otherQt+" "+newname+" libToProcess"
          print(exc)
#
#
#
def changeGlobalLinkPathForOne(f):
    deps=getGlobalDepsNoQt(f)
    for d in deps:
        shortName=getShortName(d)
        shortName="@executable_path/../lib/"+shortName
        changeSymbol(f,d,shortName)
def changeLocalLinkPathForOne(f):
    deps=getLocalDeps(f)
    for d in deps:
        shortName="@executable_path/../lib/"+re.sub("^.*\/","",d)
        changeSymbol(f,d,shortName)
def changeLibLinkPath(folder):
    for dirname, dirnames, filenames in os.walk(folder):
       for filename in filenames:
           absPath=os.path.join(dirname, filename)
           changeGlobalLinkPathForOne(absPath)
           changeLocalLinkPathForOne(absPath)
           changeQtLinkPathForOne(absPath)
           shortName="@executable_path/../lib/"+re.sub("^.*\/","",absPath)
           changeId(absPath,shortName)
#
def changeQtPlatformLinkPath(folder):
    for dirname, dirnames, filenames in os.walk(folder):
       for filename in filenames:
           absPath=os.path.join(dirname, filename)
           changeGlobalLinkPathForOne(absPath)
           changeLocalLinkPathForOne(absPath)
           changeQtLinkPathForOne(absPath)
           shortName="@executable_path/../../plugins/platforms/"+re.sub("^.*\/","",absPath)
           changeId(absPath,shortName)
def changeQtPluginLinkPath(folder):
    for dirname, dirnames, filenames in os.walk(folder):
       for filename in filenames:
           absPath=os.path.join(dirname, filename)
           changeGlobalLinkPathForOne(absPath)
           changeLocalLinkPathForOne(absPath)
           changeQtLinkPathForOne(absPath)
           shortName="@executable_path/../../plugins/imageformats/"+re.sub("^.*\/","",absPath)
           changeId(absPath,shortName)
def changeBinLinkPath(folder):
    for dirname, dirnames, filenames in os.walk(folder):
       for filename in filenames:
           absPath=os.path.join(dirname, filename)
           changeGlobalLinkPathForOne(absPath)
           changeLocalLinkPathForOne(absPath)
           changeQtLinkPathForOne(absPath)
           shortName="@executable_path/bin/"+re.sub("^.*\/","",absPath)
           changeId(absPath,shortName)
            
def changePluginLinkPath(folder,relFolder):
    for dirname, dirnames, filenames in os.walk(folder):
       for filename in filenames:
           absPath=os.path.join(dirname, filename)
           changeGlobalLinkPathForOne(absPath)
           changeQtLinkPathForOne(absPath)
           shortName="@executable_path/lib/"+relFolder+re.sub("^.*\/","",absPath)
           changeId(absPath,shortName)
#
#
#
def myChmod(dst):
        log("chmod <"+dst+">")
        cmd="/bin/chmod" + " -R 755  "+dst
        log("Exec <"+cmd+">")
        subprocess.call(cmd, shell=True)

#
def myCopy(src,dst):
    shutil.copy(src,dst)
    myChmod(dst)

#
def changeQtFileSelfLinkPath(fl,modu):
           shortName="@executable_path/../../Frameworks/"+modu+".framework/Versions/5/"+modu
           cmd="/usr/bin/install_name_tool -id "+shortName+" "+fl
           log(cmd)
           subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
def changeQtLinkPathForOne(f):
    deps=getGlobalDepsQtOnly(f)
    for d in deps:
        shortName=getShortName(d)
        shortName="@executable_path/../../Frameworks/"+shortName+".framework/Versions/5/"+shortName
        changeSymbol(f,d,shortName)
    deps2=getRpathDepsOnly(f)
    for d in deps2:
        shortName2=re.sub('@rpath','@executable_path/../../Frameworks/')
        changeSymbol(f,d,shortName2)

#
def myMkDir(target):
        if(os.path.exists(target)):
                return
        os.makedirs(target)   
#         
def myCopyTree(src,dst):
	log(src+"->"+dst)
        if(not os.path.exists(src)):
                return
        shutil.copytree(src,dst)            
        myChmod(dst)
def myCopyFile(src,dst):
        if(not os.path.exists(src)):
                return
        myCopy(src,dst)            
##
def copyQtFiles(targetFolder):
        myMkDir(targetFolder)
        for q in qts:
               print q
               if(os.path.exists(targetFolder+'/'+q+'.framework')):
                               log(targetFolder+" "+q+" already copied")
               else:
                   log("Copying"+q+' to '+targetFolder)
                   src='/opt/local/libexec/qt5/lib/'+q+'.framework'
                   dst=targetFolder+'/'+q+'.framework'
                   myMkDir(dst)
                   src=src+'/Versions/5/'
                   dst=dst+'/Versions/'
                   myMkDir(dst)
                   dst=dst+'/5/'
                   myMkDir(dst)
                   print "Copying "+dst+'Resources'
                   myCopyTree(src+'Resources',dst+'Resources')
                   print "Copying "+dst+q
                   myCopyFile(src+q,dst+q)
                   changeQtFileSelfLinkPath(dst+q,q)
                   changeQtLinkPathForOne(dst+q) 
                   changeGlobalLinkPathForOne(dst+q)
        # Also copy plugins
        myMkDir(qtPluginFolder)
	myCopyTree('/opt/local/libexec/qt5/plugins/imageformats',qtPluginFolder+'/imageformats')
	myCopyTree('/opt/local/libexec/qt5/plugins/platforms',qtPluginFolder+'/platforms')
#################################################################
# Step 1 : Copy system files so we have a standalone package
#
#################################################################
print "Copying Qt framework"
copyQtFiles(frameWorkFolder)
print "Copying Qt framework dependencies"
copyQtDeps(qts,libFolder)
print "Copying system files"
#copyFiles(binFolder,libFolder)
processed=1
# Copy file until all of them are there
while not processed == 0:
        processed=copyFiles(libFolder,libFolder)
       

