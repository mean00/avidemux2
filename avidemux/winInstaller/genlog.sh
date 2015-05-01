git log --sparse --no-decorate | grep -v "^commit" | grep -v "^Author" | grep -v "^Date" | grep -v "git-svn" | sed '/^[\t ]*$/d' | head -10 > "Change Log.html"
