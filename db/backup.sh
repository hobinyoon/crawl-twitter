#! /bin/bash

CUR_DATETIME=`date +%y%m%d-%H%H%S`
echo $CUR_DATETIME

FN_OUT="/mnt/mdc-data/pgr/twitter/backup/twitter-"$CUR_DATETIME".sql"

time mysqldump -u twitter -ptwitterpass twitter > $FN_OUT

ls -al $FN_OUT
