# Tested with gnuplot 4.6 patchlevel 4

FN_IN = system("echo $FN_IN")
FN_OUT = system("echo $FN_OUT")

set terminal pdfcairo enhanced size 4.5in, 3in
set output FN_OUT

set grid lc rgb "#808080"
set border 3 lc rgb "#808080"

set xdata time
set timefmt "%Y-%m-%d"

set xtics nomirror scale 0.5,0 font ",9" rotate by -30
set ytics nomirror scale 0.5,0 font ",9"
set yrange [0:]

plot \
FN_IN u 1:2 w linespoints pointsize 0.2 pointtype 7 not
