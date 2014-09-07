#! /bin/bash

CUR_DATETIME=`date +%y%m%d-%H%H%S`
echo $CUR_DATETIME

FN_OUT="/mnt/mdc-data/pgr/twitter/backup/twitter-"$CUR_DATETIME".sql"

time mysqldump -u twitter -ptwitterpass twitter > $FN_OUT

find /mnt/mdc-data/pgr/twitter/backup -type f -ls | sort -k 11 -r | head -n 5

echo "Strip the twitter credential table before distribution!"
