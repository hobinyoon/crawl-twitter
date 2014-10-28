# Tested with gnuplot 4.6 patchlevel 4

load "~/work/pgr/conf/colorscheme.gnuplot"

FN_IN = system("echo $FN_IN")
FN_PLOT = system("echo $FN_PLOT")

set terminal pdfcairo enhanced size 4.5in, 3in
set output FN_PLOT

set grid lc rgb "#808080"
set border 3 lc rgb "#808080"
set xlabel "R:W ratio"
set ylabel "Count"

set xtics nomirror scale 0.5,0 font ",9" format "10^%T"
set ytics nomirror scale 0.5,0 font ",9" format "10^%T"

set logscale xy

set xrange [:3000]

plot \
FN_IN u 1:2 w linespoints pointtype 7 pointsize 0.2 not
