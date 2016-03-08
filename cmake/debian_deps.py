#
#
import shutil
import re
import subprocess
import os
import sys
#
already_done = []
#
systemLibs = ["libstdc++","libm","libc","libdl","libpthread","libgcc_s","linux-vdso","libXext","libX11","/lib64/ld-linux-x86-64",
                "libXdmcp","libXau","libdrm","libXfixes","lib32z1","libxcb1"]
# Returns the list of libs as dependencies in /opt (macport)
def log(s):
    #print "Log:<"+str(s)+">"
    pass

#
def aptfilesearch(lib):
    if lib in already_done:
        return None
    cmd = ["/usr/bin/apt-file","search", lib]
    cmd = subprocess.Popen(" ".join(cmd), shell=True, stdout=subprocess.PIPE)
    for line in cmd.stdout:
        if(len(line)!=0):
            package=re.sub(":.*$","",line).strip()
            already_done.append(lib)
            return package
    return None
def getlinkedlibs(target):
    cmd = ["/usr/bin/ldd","", target]
    #log(cmd)
    q = []
    cmd = subprocess.Popen(" ".join(cmd), shell=True, stdout=subprocess.PIPE)
    for line in cmd.stdout:
       line = re.sub('=>.*$', '', line)
       line = re.sub(' ', '', line)
       line=line.strip()
       #log(line)
       q.append(line)
    #log("\n"+target + "Found deps to adjust:"+str(q))
    return q;
#
def purgeSystemLibs(folder):
   out= []
   for incoming in folder:
       short=re.sub("\.so.*$","",incoming)
       if short in systemLibs:
           pass
       else:
           out.append(incoming)
   return out
def walking(folder):
    copied=0
    dependencies = []
    #log("Copy files"+folder)
    for dirname, dirnames, filenames in os.walk(folder):
       for filename in filenames:
           absPath=os.path.join(dirname, filename)
           #log(absPath)
           libs=getlinkedlibs(absPath)
           libs=purgeSystemLibs(libs)
           log(absPath+"Found deps to adjust:"+str(libs))
           for libname in libs:
               if not libname in dependencies:
                   dependencies.append(libname)
    return dependencies
#
#print "Adjusting dependencies"
if len(sys.argv) !=1:
   print "debian_track folder"
   exit

#deps=walking('/tmp/x/usr/lib')
deps=walking(sys.argv[1])
alldeps = [ ]
for lib in deps:
   print("\t" + lib)
   package=aptfilesearch(lib)
   if package is not None:
       #print("\t"+"\t" + package)
       if not package in alldeps:
           alldeps.append(package)
print("Package dependency list:")
print("========================")
for lib in alldeps:
    print("\t"+lib) 
print("Done")
        

