# Tested with gnuplot 4.6 patchlevel 4

FN_IN = system("echo $FN_IN")
FN_OUT = system("echo $FN_OUT")
FN_WORLD_MAP = "../../worldmap/world_110m.txt"

set terminal pdfcairo enhanced size 4.5in,3in
#set terminal pngcairo enhanced size 1200,800
set output FN_OUT

set grid lc rgb "#000000"
set border 3 lc rgb "#808080"
set xtics nomirror scale 0.5,0 font ",9" format "%.0f{/Symbol \260}" autofreq -180,60
set ytics nomirror scale 0.5,0 font ",9" format "%.0f{/Symbol \260}" autofreq -90,30

set xrange[-180:180]
set yrange[-90:90]

#set label 1 "Number of requests in 0.5{/Symbol \260}-longitude/latitude-wide unit area" at 0,-65 center font ",9" front
set label 1 "Number of requests in 1{/Symbol \260}-longitude/latitude-wide unit area" at 0,-65 center font ",9" front
set object 2 rect from -110,-88 to 110,-62 fs empty border lc rgb "#D0D0D0" front

plot \
FN_WORLD_MAP with filledcurves ls 1 lc rgb "#F0F0F0" not, \
FN_IN u 1:2:3 with points pt 6 ps variable lc rgb "#FF0000" lw 0.4 not, \
FN_IN u 1:($2-10):4 with labels tc rgb "#000000" font ",7" not
