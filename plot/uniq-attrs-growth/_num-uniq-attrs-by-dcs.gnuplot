# Tested with gnuplot 4.6 patchlevel 4

load "~/work/pgr/conf/colorscheme.gnuplot"

FN_IN = system("echo $FN_IN")
FN_PLOT = system("echo $FN_PLOT")

set terminal pdfcairo enhanced size 4.5in, 3in
set output FN_PLOT

set grid lc rgb "#808080"
set border 3 lc rgb "#808080"
set ylabel "# of unique videos, users, and topics" font ",9"

set xtics nomirror scale 0.5,0 font ",9"
set ytics nomirror scale 0.5,0 font ",9" format "%.0fK"

set style fill solid 0.5 noborder
BOXWIDTH=0.16

plot \
FN_IN u ($0-0.2):($2/1000):(BOXWIDTH) w boxes t "videos", \
FN_IN u ($0)    :($3/1000):(BOXWIDTH) w boxes t "users", \
FN_IN u ($0+0.2):($4/1000):(BOXWIDTH) w boxes t "topics", \
FN_IN u 0:xticlabel(1) with points ps 0 not
