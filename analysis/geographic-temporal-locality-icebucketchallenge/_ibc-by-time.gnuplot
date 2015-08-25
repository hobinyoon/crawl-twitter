# Tested with gnuplot 4.6 patchlevel 4

load "~/work/coco/conf/colorscheme.gnuplot"

FN_IN = system("echo $FN_IN")
FN_PLOT = system("echo $FN_PLOT")

set terminal pdfcairo enhanced size 2in, 1.75in
set output FN_PLOT

set lmargin at screen 0.14
set rmargin at screen 0.992
set tmargin at screen 0.97
set bmargin at screen 0.188

set border 3 lc rgb "#808080"
set xlabel "Date" offset 0,0.3
set xtics nomirror scale 0.5,0 autofreq 0,30*24*3600 font ",10" rotate by -15
#autofreq 24*3600,2*24*3600
set ytics nomirror scale 0.5,0 autofreq 0,300

set timefmt "%Y-%m-%d"
set xdata time
set format x "%b-%d"

set xrange ["2014-07-01":"2014-10-31"]

set style fill solid 0.6 noborder
BOXWIDTH=0.8*24*3600

plot \
FN_IN u 1:10:(BOXWIDTH) w boxes not
