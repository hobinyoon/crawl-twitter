#!/usr/bin/gnuplot

FN_IN = "get-youtube-video-details/video-detailed-merged"
FN_OUT = "numtweets-vs-viewcnt.pdf"

set print "-"
#print sprintf("FN_IN=%s", FN_IN)
#print sprintf("FN_OUT=%s", FN_OUT)

if (1) {
	set terminal unknown
	plot FN_IN u 2:3 w p
	X_MIN=GPVAL_DATA_X_MIN
	X_MAX=GPVAL_DATA_X_MAX
	Y_MIN=GPVAL_DATA_Y_MIN
	Y_MAX=GPVAL_DATA_Y_MAX
}

set terminal pdfcairo enhanced size 2.3in, (2.3*0.85)in
set output FN_OUT

set border front lc rgb "#808080" back
set xtics nomirror tc rgb "black" format "10^%T"
set ytics nomirror tc rgb "black" format "10^%T"
set tics front
set grid xtics ytics back lc rgb "#808080"

set xlabel "Num tweets"
set ylabel "View count"

set logscale xy

set xrange[:X_MAX]
set yrange[:Y_MAX]

plot FN_IN u 2:3 w p pt 7 pointsize 0.15 lc rgb "red" not

print sprintf("Created %s", FN_OUT)
