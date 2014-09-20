#! /bin/bash

set -e
set -u

CUR_DATETIME=`date +%y%m%d-%H%M%S`
echo $CUR_DATETIME

FN_OUT="/mnt/mdc-data/pgr/twitter/backup/twitter-"$CUR_DATETIME".sql"

time /usr/bin/mysqldump --skip-tz-utc -u twitter -ptwitterpass twitter > $FN_OUT

/usr/bin/7z a -mx=9 $FN_OUT".7z" $FN_OUT
\rm $FN_OUT

/usr/bin/find /mnt/mdc-data/pgr/twitter/backup -type f -ls | /usr/bin/sort -k 11 -r | /usr/bin/head -n 5

echo "\nStrip the twitter credential table before distribution!"
