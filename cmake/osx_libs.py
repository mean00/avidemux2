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
qts = ['QtCore', 'QtGui', 'QtOpenGl']

#
#
allSymbols=[]
#
#
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
       if(not line.startswith('/opt/local')):
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
    return q;
#
def getGlobalDepsQtOnly(target):
    q= []
    p = getGlobalDeps(target)
    for line in p:
       if getShortName(line).startswith('Qt'):
          q.append(line)
    return q;
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
                   shutil.copy(dep,libFolder)
                   copied+=1
    return copied
#
# 
# Copy used library coming from /opt/local
#
def copyQtDeps(components,libFolder):
    copied=0
    for modul in components:
        absPath='/opt/local/Library/Frameworks/'+modul+'.framework/Versions/4/'+modul
        print("Copy deps for "+modul+" ("+absPath+")")
        deps=getGlobalDepsNoQt(absPath)
        for dep in deps:
                   shortName=re.sub('^.*\/','',dep)
                   if(os.path.exists(libFolder+'/'+shortName)):
                               print(shortName+" already copied")
                   else:
                       print("Copying:"+shortName)
                       shutil.copy(dep,libFolder)
                       copied+=1
    # copy plugins deps too
    copyFiles(qtPluginFolder+'/imageformats',libFolder)
    return copied
##
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
        cmd="/usr/bin/install_name_tool -change "+d+" "+shortName+" "+f
        log(cmd)
        subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
def changeLocalLinkPathForOne(f):
    deps=getLocalDeps(f)
    for d in deps:
        shortName="@executable_path/../lib/"+re.sub("^.*\/","",d)
        cmd="/usr/bin/install_name_tool -change "+d+" "+shortName+" "+f
        log(cmd)
        subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
              
def changeLibLinkPath(folder):
    for dirname, dirnames, filenames in os.walk(folder):
       for filename in filenames:
           absPath=os.path.join(dirname, filename)
           changeGlobalLinkPathForOne(absPath)
           changeLocalLinkPathForOne(absPath)
           changeQtLinkPathForOne(absPath)
           shortName="@executable_path/../lib/"+re.sub("^.*\/","",absPath)
           cmd="/usr/bin/install_name_tool -id "+shortName+" "+absPath
           log(cmd)
           subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
def changeQtPluginLinkPath(folder):
    for dirname, dirnames, filenames in os.walk(folder):
       for filename in filenames:
           absPath=os.path.join(dirname, filename)
           changeGlobalLinkPathForOne(absPath)
           changeLocalLinkPathForOne(absPath)
           changeQtLinkPathForOne(absPath)
           shortName="@executable_path/../../plugins/imageformats/"+re.sub("^.*\/","",absPath)
           cmd="/usr/bin/install_name_tool -id "+shortName+" "+absPath
           log(cmd)
           subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
def changeBinLinkPath(folder):
    for dirname, dirnames, filenames in os.walk(folder):
       for filename in filenames:
           absPath=os.path.join(dirname, filename)
           changeGlobalLinkPathForOne(absPath)
           changeLocalLinkPathForOne(absPath)
           changeQtLinkPathForOne(absPath)
           shortName="@executable_path/bin/"+re.sub("^.*\/","",absPath)
           cmd="/usr/bin/install_name_tool -id "+shortName+" "+absPath
           log(cmd)
           subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
            
def changePluginLinkPath(folder,relFolder):
    for dirname, dirnames, filenames in os.walk(folder):
       for filename in filenames:
           absPath=os.path.join(dirname, filename)
           changeGlobalLinkPathForOne(absPath)
           changeQtLinkPathForOne(absPath)
           shortName="@executable_path/lib/"+relFolder+re.sub("^.*\/","",absPath)
           cmd="/usr/bin/install_name_tool -id "+shortName+" "+absPath
           log(cmd)
           subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
#
#
#
def changeQtFileSelfLinkPath(fl,modu):
           shortName="@executable_path/../../Frameworks/"+modu+".framework/Versions/4/"+modu
           cmd="/usr/bin/install_name_tool -id "+shortName+" "+fl
           log(cmd)
           subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
def changeQtLinkPathForOne(f):
    deps=getGlobalDepsQtOnly(f)
    for d in deps:
        shortName=getShortName(d)
        shortName="@executable_path/../../Frameworks/"+shortName+".framework/Versions/4/"+shortName
        cmd="/usr/bin/install_name_tool -change "+d+" "+shortName+" "+f
        log(cmd)
        subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)

#
def myMkDir(target):
        if(os.path.exists(target)):
                return
        os.makedirs(target)   
#         
def myCopyTree(src,dst):
        if(not os.path.exists(src)):
                return
        shutil.copytree(src,dst)            
def myCopyFile(src,dst):
        if(not os.path.exists(src)):
                return
        shutil.copy(src,dst)            
##
def copyQtFiles(targetFolder):
        myMkDir(targetFolder)
        for q in qts:
               print q
               if(os.path.exists(targetFolder+'/'+q+'.framework')):
                               log(q+" already copied")
               else:
                   log("Copying"+q+' to '+targetFolder)
                   src='/opt/local/Library/Frameworks/'+q+'.framework'
                   dst=targetFolder+'/'+q+'.framework'
                   myMkDir(dst)
                   src=src+'/Versions/4/'
                   dst=dst+'/Versions/'
                   myMkDir(dst)
                   dst=dst+'/4/'
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
        myCopyTree('/opt/local/share/qt4/plugins/imageformats',qtPluginFolder+'/imageformats')
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
#################################################################
# Step 2 :  Change link name so that they are all executable_path
#               relative
#################################################################
print "Adjusting dependencies"
changeBinLinkPath(binFolder)
changeLibLinkPath(libFolder)
changeQtPluginLinkPath(qtPluginFolder)
subFolders=["audioDecoder",    "audioEncoders",   "autoScripts",     "demuxers",        "muxers",          "scriptEngines",   "videoEncoders",   "videoFilters"]
for s in subFolders:
        relFolder="ADM_plugins6/"+s
        changePluginLinkPath(libFolder+"/"+relFolder,relFolder)
        

