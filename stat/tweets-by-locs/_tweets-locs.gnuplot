# Tested with gnuplot 4.6 patchlevel 4

FN_IN = system("echo $FN_IN")
FN_OUT = system("echo $FN_OUT")
FN_DC_LOC = "~/work/pgr/conf/dc-coord"

set terminal pdfcairo enhanced size 4.5in,3in
set output FN_OUT

set grid lc rgb "#C0C0C0"
set noborder
set xtics nomirror scale 0,0 font ",10" format "%.0f{/Symbol \260}" tc rgb "#808080" autofreq -180,60
set ytics nomirror scale 0,0 font ",10" format "%.0f{/Symbol \260}" tc rgb "#808080" autofreq -90,30

set xrange[-140:140]
set yrange[-70:70]

#set label 1 "Number of requests in 0.5{/Symbol \260}-longitude/latitude-wide unit area" at -60,20 center font ",9" front
#set object 2 rect from -110,-88 to 110,-62 fs empty border lc rgb "#D0D0D0" front

plot \
'~/work/pgr/resource/world_110m.txt' with filledcurves fs transparent solid 0.15 noborder lc rgb "#C0C0C0" not, \
FN_IN     u 1:2:4 with circles fs transparent solid 0.35 noborder lc rgb "#FF0000" lw 0.4 not, \
FN_IN     u 1:($2-8.0):5 with labels tc rgb "#000000" font ",8" not, \
FN_DC_LOC u 3:2:(2.0) with circles lt 3 fs solid 1.0 noborder not, \
FN_DC_LOC u 3:2:1 with labels offset 0,-0.5 tc rgb "#0000FF" font ",9" not
