#!/bin/bash

set -e
set -u

SRC_DIR=`dirname $BASH_SOURCE`

export FN_IN=$SRC_DIR/data/youtube-op-locs
export FN_OUT=$SRC_DIR/youtube-op-locs.pdf
gnuplot $SRC_DIR/_tweets-locs.gnuplot 2>&1 | sed 's/^/  /'
if [ "${PIPESTATUS[0]}" -ne "0" ]; then
	exit 1
fi
printf "Created %s %d\n" $FN_OUT `wc -c < $FN_OUT`
