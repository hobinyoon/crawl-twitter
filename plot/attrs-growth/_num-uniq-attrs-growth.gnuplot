# Tested with gnuplot 4.6 patchlevel 4

load "~/work/pgr/conf/colorscheme.gnuplot"

FN_IN = system("echo $FN_IN")
FN_PLOT = system("echo $FN_PLOT")

set terminal pdfcairo enhanced size 4.5in, 3in
set output FN_PLOT

set xlabel "# of YouTube videos"
set ylabel "# of attrs / # of videos" offset 1,0
set grid lc rgb "#808080"
set border 3 lc rgb "#808080"

set xtics nomirror scale 0.5,0 ( \
   "0"      0, \
 "25K"  25000, \
 "50K"  50000, \
 "75K"  75000, \
"100K" 100000, \
"125K" 125000, \
"150K" 150000 \
)
set ytics nomirror scale 0.5,0

set yrange [0:]

plot \
FN_IN u 1:($4/$1) w lines t "users", \
FN_IN u 1:($6/$1) w lines t "topics"

# set pointsize 0.2
# FN_IN u 1:($4/$1) w linespoints pt 7 t "users", \
# FN_IN every 1000::1000 u 1:($4/$1) w linespoints pt 7 t "topics", \
