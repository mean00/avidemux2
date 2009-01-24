svn log --stop-on-copy --xml ../.. > svn.xml
xsltproc svnlog.xslt svn.xml > "../../../avidemux_2.4_build/Change Log.html"
xsltproc revision.xslt svn.xml > revision.nsh
rm svn.xml
