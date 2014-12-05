# Tested with gnuplot 4.6 patchlevel 4

load "~/work/pgr/conf/colorscheme.gnuplot"

FN_IN = system("echo $FN_IN")
FN_PLOT = system("echo $FN_PLOT")

set terminal pdfcairo enhanced size 4.5in, 3in
set output FN_PLOT

set grid lc rgb "#808080"
set noborder
set xtics nomirror scale 0,0 font ",9" format "%.0f{/Symbol \260}" tc rgb "#808080" autofreq -180,20
set ytics nomirror scale 0,0 font ",9" format "%.0f{/Symbol \260}" tc rgb "#808080" autofreq -90,10

set xrange[-20:40]
set yrange[30:60]

plot \
'~/work/pgr/resource/world_110m.txt' with filledcurves fs transparent solid 0.20 noborder lc rgb "#C0C0C0" not, \
FN_IN u 1:2:3 with circles fs transparent solid 0.35 noborder lc rgb "#FF0000" lw 0.4 not, \
FN_IN u 1:2:5 with labels offset 0,-1.0 tc rgb "#000000" font ",8" not
