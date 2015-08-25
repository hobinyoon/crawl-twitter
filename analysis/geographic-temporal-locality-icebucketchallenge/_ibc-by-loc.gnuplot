# Tested with gnuplot 4.6 patchlevel 4

load "~/work/coco/conf/colorscheme.gnuplot"

FN_IN = system("echo $FN_IN")
FN_PLOT = system("echo $FN_PLOT")

set terminal pdfcairo enhanced size 2in, 1.75in
set output FN_PLOT

set multiplot

set lmargin at screen 0.10
set rmargin at screen 0.992
set tmargin at screen 1.0
set bmargin at screen 0.245

set grid lc rgb "#C0C0C0"
set noborder
set xtics nomirror scale 0,0 format "%.0f{/Symbol \260}" tc rgb "#808080" autofreq 0,20,20  offset 0,0.2
set ytics nomirror scale 0,0 format "%.0f{/Symbol \260}" tc rgb "#808080" autofreq 40,10,50 offset 1,0

set xrange[-20:40]
set yrange[35:60]

plot \
'~/work/coco/resource/world_110m.txt' with filledcurves fs transparent solid 0.25 noborder lc rgb "#C0C0C0" not

reset

set lmargin at screen 0.10
set rmargin at screen 0.992
set tmargin at screen 1.0
set bmargin at screen 0.095

set noborder
set xtics nomirror scale 0,0 autofreq 1000,1000 offset 0,0.2
set ytics nomirror scale 0,0 autofreq 1000,1000 offset 1,0

set xrange[-20:40]
set yrange[30:60]

plot \
FN_IN u 1:2:3 with circles fs transparent solid 0.35 noborder lc rgb "#FF0000" lw 0.4 not, \
FN_IN u 1:2:5 with labels offset 0,-1.0 tc rgb "#000000" not

#'~/work/coco/resource/world_110m.txt' with filledcurves fs transparent solid 0.25 noborder lc rgb "#C0C0C0" not, \
