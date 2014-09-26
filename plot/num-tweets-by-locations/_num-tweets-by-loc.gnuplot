# Tested with gnuplot 4.6 patchlevel 4

FN_IN = system("echo $FN_IN")
FN_OUT = system("echo $FN_OUT")

set terminal pngcairo enhanced size 1200,800
set output FN_OUT

set grid lc rgb "#808080"
set border 3 lc rgb "#808080"
set xtics nomirror scale 0.5,0 font ",9" autofreq -180,20
set ytics nomirror scale 0.5,0 font ",9" autofreq -90,10

set xrange[-180:180]
set yrange[-90:90]

plot \
'world_10m.txt' with filledcurves ls 1 lc rgb "#F0F0F0" not, \
FN_IN u 1:2:($3*0.1) with points pt 6 ps variable lc rgb "#FF0000" t "# of tweets"
