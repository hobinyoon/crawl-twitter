#! /bin/bash

set -e
set -u

DB_NAME=twitter4
CUR_DATETIME=`date +%y%m%d-%H%M%S`
echo $CUR_DATETIME

DN_BACKUP=~/work/crawl-twitter-data/backup

FN_SQL=$DN_BACKUP"/"$DB_NAME"-"$CUR_DATETIME".sql"
FN_SQL_7Z=$FN_SQL".7z"

# --skip-lock-tables to prevent deadlock. I've seen deadlocks on the credentials table many times without this option.
time /usr/bin/mysqldump --skip-tz-utc --skip-lock-tables -u twitter $DB_NAME > $FN_SQL

/usr/bin/7z a -mx=9 $FN_SQL_7Z $FN_SQL
\rm $FN_SQL

/usr/bin/find $DN_BACKUP -type f -ls | /usr/bin/sort -k 11 -r | /usr/bin/head -n 5

time /usr/bin/aws s3 sync ~/work/crawl-twitter-data/backup s3://youtube-accesses

echo
echo "Warning: keep in mind that it contains the Twitter credential data that you want to keep private!"
