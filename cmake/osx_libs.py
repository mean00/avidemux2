#
#
import shutil
import re
import subprocess
import os
print "Preparing independant bundle..."
rootFolder="/Users/fx/Avidemux2.6.app/Contents/Resources"
libFolder=rootFolder+"/lib"
binFolder=rootFolder+"/bin"

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
# Copy Qt4 libraries, add them to the pool of symbols + to the pool of 
# file to process
#
def copyFiles(folder,libFolder):
    copied=0
    log("Copy files"+folder)
    for dirname, dirnames, filenames in os.walk(folder):
       for filename in filenames:
           absPath=os.path.join(dirname, filename)
           deps=getGlobalDeps(absPath)
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
    deps=getGlobalDeps(f)
    for d in deps:
        shortName="@executable_path/../lib/"+re.sub("^.*\/","",d)
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
           shortName="@executable_path/../lib/"+re.sub("^.*\/","",absPath)
           cmd="/usr/bin/install_name_tool -id "+shortName+" "+absPath
           log(cmd)
           subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
def changeBinLinkPath(folder):
    for dirname, dirnames, filenames in os.walk(folder):
       for filename in filenames:
           absPath=os.path.join(dirname, filename)
           changeGlobalLinkPathForOne(absPath)
           changeLocalLinkPathForOne(absPath)
           shortName="@executable_path/bin/"+re.sub("^.*\/","",absPath)
           cmd="/usr/bin/install_name_tool -id "+shortName+" "+absPath
           log(cmd)
           subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
            
def changePluginLinkPath(folder,relFolder):
    for dirname, dirnames, filenames in os.walk(folder):
       for filename in filenames:
           absPath=os.path.join(dirname, filename)
           changeGlobalLinkPathForOne(absPath)
           shortName="@executable_path/lib/"+relFolder+re.sub("^.*\/","",absPath)
           cmd="/usr/bin/install_name_tool -id "+shortName+" "+absPath
           log(cmd)
           subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
            


#################################################################
# Step 1 : Copy system files so we have a standalone package
#
#################################################################
copyFiles(binFolder,libFolder)
processed=1
# Copy file until all of them are there
while not processed ==0:
        processed=copyFiles(libFolder,libFolder)
#################################################################
# Step 2 :  Change link name so that they are all executable_path
#               relative
#################################################################
changeBinLinkPath(binFolder)
changeLibLinkPath(libFolder)

subFolders=["audioDecoder",    "audioEncoders",   "autoScripts",     "demuxers",        "muxers",          "scriptEngines",   "videoEncoders",   "videoFilters"]
for s in subFolders:
        relFolder="ADM_plugins6/"+s
        changePluginLinkPath(libFolder+"/"+relFolder,relFolder)
        

