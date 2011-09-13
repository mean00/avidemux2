svn log --stop-on-copy --xml $SOURCEDIR > "svn [$BUILDBITS].xml"
xsltproc svnlog.xslt "svn [$BUILDBITS].xml" > "$BUILDDIR/Change Log.html"
xsltproc revision.xslt "svn [$BUILDBITS].xml" > revision.nsh
rm "svn [$BUILDBITS].xml"
