# Tested with gnuplot 4.6 patchlevel 4

load "~/work/pgr/conf/colorscheme.gnuplot"

FN_IN = system("echo $FN_IN")
FN_PLOT = system("echo $FN_PLOT")

set terminal pdfcairo enhanced size 4.5in, 3in
set output FN_PLOT

set grid ytics lc rgb "#808080"
set border 3 lc rgb "#808080"
set xlabel "Date (Aug. 2014)"
set ylabel "# of videos with topic IceBucketChallenge" offset 1.7,0
set xtics nomirror scale 0.5,0 font ",9" autofreq 24*3600,2*24*3600
set ytics nomirror scale 0.5,0 font ",9"

set timefmt "%Y-%m-%d"
set xdata time
#set format x "%m-%d"
set format x "%d"

set xrange ["2014-08-07":"2014-09-01"]

set style fill solid 0.4 noborder
BOXWIDTH=0.6*24*3600

plot \
FN_IN u 1:10:(BOXWIDTH) w boxes not
