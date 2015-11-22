export outFile="ChangeLog.html"
echo "<html><head><link rel="stylesheet" type="text/css" href="change.css"></head>" > $outFile
echo "<body>Avidemux Changelog:<br>" >> $outFile
echo "<table><tr><th>Message</th><th>Commit</th></tr>" >> $outFile
git log  --pretty=format:'<tr><td> %s </td><td><a href="https://github.com/mean00/avidemux2/commit/%H"> commit</td></tr>' | head -20   >> $outFile

echo "</body>" >> $outFile
