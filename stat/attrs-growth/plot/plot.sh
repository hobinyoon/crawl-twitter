#!/bin/bash

set -e
set -u

SRC_DIR=`dirname $BASH_SOURCE`

echo "Plotting uniq attrs growth ..."
export FN_IN=$SRC_DIR/data/num-uniq-attrs-growth
export FN_OUT=$FN_IN".pdf"
gnuplot $SRC_DIR/_num-uniq-attrs-growth.gnuplot 2>&1 | sed 's/^/  /'
if [ "${PIPESTATUS[0]}" -ne "0" ]; then
	exit 1
fi
printf "  Created %s %d\n" $FN_OUT `wc -c < $FN_OUT`

echo "Plotting uniq users growth by DCs ..."
export FN_IN=$SRC_DIR/data/num-uniq-attrs-growth-by-dcs
export FN_OUT=$SRC_DIR/data/num-uniq-users-growth-by-dcs.pdf
gnuplot $SRC_DIR/_num-uniq-users-growth-by-dcs.gnuplot 2>&1 | sed 's/^/  /'
if [ "${PIPESTATUS[0]}" -ne "0" ]; then
	exit 1
fi
printf "  Created %s %d\n" $FN_OUT `wc -c < $FN_OUT`

echo "Plotting uniq topics growth by DCs ..."
export FN_IN=$SRC_DIR/data/num-uniq-attrs-growth-by-dcs
export FN_OUT=$SRC_DIR/data/num-uniq-topics-growth-by-dcs.pdf
gnuplot $SRC_DIR/_num-uniq-topics-growth-by-dcs.gnuplot 2>&1 | sed 's/^/  /'
if [ "${PIPESTATUS[0]}" -ne "0" ]; then
	exit 1
fi
printf "  Created %s %d\n" $FN_OUT `wc -c < $FN_OUT`
