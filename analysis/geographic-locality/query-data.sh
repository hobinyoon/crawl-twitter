#!/bin/bash

set -e
set -u

#hashtags=(tennis usopen wimbledon frenchopen aussieopen bbnt)
hashtags=(usopen wimbledon frenchopen aussieopen bbnt)

for ht in ${hashtags[@]}; do
	echo $ht
	time mysql -utwitter -ptwitterpass -e "select * from tweets where hashtags like '%"$ht"%'" twitter3 > hashtag-$ht
done
