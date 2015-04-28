#! /bin/bash

set -e
set -u

DB_NAME=twitter3
CUR_DATETIME=`date +%y%m%d-%H%M%S`
echo $CUR_DATETIME

FN_OUT="/mnt/data/twitter/backup/"$DB_NAME"-"$CUR_DATETIME".sql"

time /usr/bin/mysqldump --skip-tz-utc -u twitter -ptwitterpass $DB_NAME > $FN_OUT

/usr/bin/7z a -mx=9 $FN_OUT".7z" $FN_OUT
\rm $FN_OUT

/usr/bin/find /mnt/data/twitter/backup -type f -ls | /usr/bin/sort -k 11 -r | /usr/bin/head -n 5

echo
echo "Strip the credential table before distribution!"
