# Tested with gnuplot 4.6 patchlevel 4

FN_IN = system("echo $FN_IN")
FN_OUT = system("echo $FN_OUT")

set terminal pdfcairo enhanced size 4.5in,3in
set output FN_OUT

set grid lc rgb "#000000"
set border 3 lc rgb "#808080"
set xtics nomirror scale 0.5,0 font ",9" format "%.0f{/Symbol \260}" autofreq -180,10
set ytics nomirror scale 0.5,0 font ",9" format "%.0f{/Symbol \260}" autofreq -90,10

set xrange[-120:0]
set yrange[10:70]

set label 1 "Number of requests in 0.5{/Symbol \260}-longitude/latitude-wide unit area" at -60,20 center font ",9" front
#set object 2 rect from -110,-88 to 110,-62 fs empty border lc rgb "#D0D0D0" front

plot \
'~/work/pgr/resource/world_110m.txt' with filledcurves ls 1 lc rgb "#F0F0F0" not, \
FN_IN u 1:2:4 with points pt 6 ps variable lc rgb "#FF0000" lw 0.4 not, \
FN_IN u 1:($2-3.5):5 with labels tc rgb "#000000" font ",7" not
