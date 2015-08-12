#! /bin/bash

set -e
set -u

DB_NAME=twitter3
CUR_DATETIME=`date +%y%m%d-%H%M%S`
echo $CUR_DATETIME

FN_SQL="/mnt/data/twitter/backup/"$DB_NAME"-"$CUR_DATETIME".sql"
FN_SQL_7Z=$FN_SQL".7z"

time /usr/bin/mysqldump --skip-tz-utc -u twitter -ptwitterpass $DB_NAME > $FN_SQL

/usr/bin/7z a -mx=9 $FN_SQL_7Z $FN_SQL
\rm $FN_SQL

/usr/bin/find /mnt/data/twitter/backup -type f -ls | /usr/bin/sort -k 11 -r | /usr/bin/head -n 5

#REMOTE_MACHINE="143.215.207.89"
REMOTE_MACHINE="130.207.110.227"
echo
echo "Remote backup to "$REMOTE_MACHINE
time scp -p $FN_SQL_7Z hobin@$REMOTE_MACHINE:archive/backup/twitter/

echo
echo "Strip the credential table before distribution!"
