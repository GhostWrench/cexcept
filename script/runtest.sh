#/bin/sh
valgrind --track-origins=yes --leak-check=yes --log-file=$2 $1
if [ $? = 0 ];
then
    echo "$(basename $1 .elf) .. OK"
else
    echo "$(basename $1 .elf) .. FAILED"
fi
exit 0
