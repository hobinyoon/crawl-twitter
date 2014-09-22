# Tested with gnuplot 4.6 patchlevel 4

FN_IN = system("echo $FN_IN")
FN_OUT = system("echo $FN_OUT")

set terminal pngcairo enhanced size 1200,800
set output FN_OUT

set grid lc rgb "#808080"
set border 3 lc rgb "#808080"

set xtics nomirror scale 0.5,0 font ",9"
set ytics nomirror scale 0.5,0 font ",9"

plot \
FN_IN u 1:2 w points pointsize 0.1 pointtype 7 not
