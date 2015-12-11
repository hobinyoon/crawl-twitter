#!/bin/bash

set -e
set -u

SRC_DIR=`dirname $BASH_SOURCE`

Plot() {
	export FN_IN=$SRC_DIR/../data/$HT-by-time
	export FN_OUT=$SRC_DIR/temporal-locality-$HT.pdf
	gnuplot $SRC_DIR/_temporal.gnuplot
	printf "Created %s %d\n" $FN_OUT `wc -c < $FN_OUT`
}

#hashtags=(tennis usopen wimbledon frenchopen aussieopen)
#hashtags=(usopen wimbledon frenchopen aussieopen)
hashtags=(wimbledon)
for ht1 in ${hashtags[@]}; do
	HT=$ht1
	./bucketize.py 7
	Plot
done
