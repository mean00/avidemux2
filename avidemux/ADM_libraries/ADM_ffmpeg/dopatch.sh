#

for i in patches/*.patch ; do
        echo $i
        patch -p0  < $i
done


