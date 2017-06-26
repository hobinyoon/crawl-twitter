#! /bin/bash

set -e
set -u

DB_NAME=twitter4
CUR_DATETIME=`date +%y%m%d-%H%M%S`
echo $CUR_DATETIME

DN_BACKUP=~/work/crawl-twitter-data/backup

FN_SQL=$DN_BACKUP"/"$DB_NAME"-"$CUR_DATETIME".sql"
FN_SQL_7Z=$FN_SQL".7z"

time /usr/bin/mysqldump --skip-tz-utc -u twitter $DB_NAME > $FN_SQL

/usr/bin/7z a -mx=9 $FN_SQL_7Z $FN_SQL
\rm $FN_SQL

/usr/bin/find $DN_BACKUP -type f -ls | /usr/bin/sort -k 11 -r | /usr/bin/head -n 5

echo
echo "Strip the credential table before distribution!"
