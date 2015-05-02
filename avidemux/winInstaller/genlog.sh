export output="Change Log.html"
echo "<html><head></head><body><h1>Changelog:</h1><br>" > $output
git log  --pretty=format:'<li> %s <a href="https://github.com/mean00/avidemux2/commit/%H"> *;</a> </li> ' | head -20   >> $output

echo "</body>" >> $output
