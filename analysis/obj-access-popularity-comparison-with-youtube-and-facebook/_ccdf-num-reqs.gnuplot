# Tested with gnuplot 4.6 patchlevel 4

FN_IN = system("echo $FN_IN")
FN_OUT = system("echo $FN_OUT")
FN_YAW = system("echo $FN_YAW")

load "~/work/pgr/conf/colorscheme.gnuplot"

set terminal pdfcairo enhanced size 4.5in, 3in
set output FN_OUT
set xlabel "Logarithm of number of requests"
set ylabel "Logarithm of CCDF"
set border (1 + 2) lc rgb "#C0C0C0"
set grid xtics ytics back lt 0 lc rgb "#C0C0C0"

set xtics nomirror scale 1.0,0.5 tc rgb "#0000FF"
set mxtics 10
set ytics nomirror scale 1.0,0.5 tc rgb "#0000FF"
set mytics 10

set logscale xy
set format xy "10^{%L}"

set xrange [:2357]
set yrange [1.97*(10**(-6)) : 1]
set y2range [0.09:1.074]

set pointsize 0.7

plot \
FN_YAW u 1:2 axes x2y2 w points pt 6 t "Brodersen 2012", \
FN_IN  u 1:(1-$2) w points pt 6 t "Our dataset"
