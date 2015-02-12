# Tested with gnuplot 4.6 patchlevel 4

FN_IN = system("echo $FN_IN")
FN_OUT = system("echo $FN_OUT")

# load "~/work/pgr/conf/colorscheme.gnuplot"

set terminal pdfcairo enhanced size 4.5in, 3in
set output FN_OUT
set xlabel "Popularity ranking"
set ylabel "Number of access"
set border (1 + 2) lc rgb "#C0C0C0"
set grid xtics ytics back lt 0 lc rgb "#C0C0C0"
set xtics nomirror scale 1.0,0.5 tc rgb "#202020"
set mxtics 10
set ytics nomirror scale 1.0,0.5 tc rgb "#202020"
set mytics 10

set logscale xy
set format xy "10^{%L}"
set xrange [:10000]

plot \
FN_IN u 0:1 w lines not
