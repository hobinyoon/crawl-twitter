# Tested with gnuplot 4.6 patchlevel 4

load "~/work/pgr/conf/colorscheme.gnuplot"

FN_IN = system("echo $FN_IN")
FN_PLOT = system("echo $FN_PLOT")

set terminal pdfcairo enhanced size 4.5in, 3in
set output FN_PLOT

set grid lc rgb "#808080"
set border 3 lc rgb "#808080"
set xtics nomirror scale 0.5,0 font ",9" format "%.0f{/Symbol \260}" autofreq -180,10
set ytics nomirror scale 0.5,0 font ",9" format "%.0f{/Symbol \260}" autofreq -90,10

#set xrange[-180:180]
#set yrange[-90:90]
set xrange[-20:40]
set yrange[30:60]

# 
# set label 1 "Number of requests in 0.5{/Symbol \260}-longitude/latitude-wide unit area" at -60,20 center font ",9" front
# #set object 2 rect from -110,-88 to 110,-62 fs empty border lc rgb "#D0D0D0" front
# 
plot \
'~/work/pgr/resource/world_110m.txt' with filledcurves ls 1 lc rgb "#F0F0F0" not, \
FN_IN u 1:2:3 with points pt 6 ps variable lc rgb "#FF0000" lw 0.4 not, \
FN_IN u 1:2:5 with labels offset 0,-0.8 tc rgb "#000000" font ",8" not
