# Tested with gnuplot 4.6 patchlevel 4

FN_IN = system("echo $FN_IN")
FN_OUT = system("echo $FN_OUT")

set terminal pdfcairo enhanced size 4.5in, 3in
set output FN_OUT

set grid ytics front lc rgb "#000000"
set border 3 lc rgb "#808080"

set title "# of users crawled per hour"
set xdata time
set timefmt "%Y%m%d%H"
set format x "%m/%d"

set xtics nomirror scale 0.5,0 font ",9"
set ytics nomirror scale 0.5,0 font ",9"

set style fill solid 0.5 noborder
BOX_WIDTH=3600
BOX_SPACING=0.1
plot \
FN_IN u 1:2:(BOX_WIDTH) w boxes not
