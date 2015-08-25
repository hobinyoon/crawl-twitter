#!/bin/bash

set -e
set -u

SRC_DIR=`dirname $BASH_SOURCE`

HT=tennis

Cluster() {
	pushd $SRC_DIR/cluster-locations/.build > /dev/null
	make -j
	popd > /dev/null
	$SRC_DIR/cluster-locations/.build/cluster-locations \
		--fn_input=data/hashtag-$HT \
		--topic=$HT \
		--month_date_0=01-01 \
		--month_date_1=06-07 \
		--cluster_dist=2.0 \
		--fn_output=data/clustered-$HT
	echo
}

Plot() {
	export FN_IN=$SRC_DIR/data/clustered-$HT
	export FN_OUT=$SRC_DIR/geo-locality-$HT.pdf
	gnuplot $SRC_DIR/_geographic.gnuplot
	echo "Created "$FN_OUT
}

#hashtags=(tennis usopen wimbledon frenchopen aussieopen)
#hashtags=(usopen wimbledon frenchopen aussieopen)
hashtags=(wimbledon)
for ht1 in ${hashtags[@]}; do
	HT=$ht1
	Cluster
	Plot
done
