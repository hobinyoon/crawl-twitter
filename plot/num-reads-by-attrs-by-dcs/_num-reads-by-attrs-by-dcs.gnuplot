# Tested with gnuplot 4.6 patchlevel 4

load "~/work/pgr/conf/colorscheme.gnuplot"

FN_BY_VIDS = system("echo $FN_BY_VIDS")
FN_BY_UIDS = system("echo $FN_BY_UIDS")
FN_BY_TOPICS = system("echo $FN_BY_TOPICS")

FN_PLOT = system("echo $FN_PLOT")

set terminal pdfcairo enhanced size 4.5in, 3in
set output FN_PLOT

set grid lc rgb "#808080"
set border 3 lc rgb "#808080"

set xtics nomirror scale 0.5,0 font ",9"
set ytics nomirror scale 0.5,0 font ",9"

set logscale x
set xrange [:1000]

plot \
FN_BY_VIDS   u 1:2 w lines t "per video per DC", \
FN_BY_UIDS   u 1:2 w lines t "per user per DC", \
FN_BY_TOPICS u 1:2 w lines t "per topic per DC"
