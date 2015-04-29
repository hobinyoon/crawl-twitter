# Tested with gnuplot 4.6 patchlevel 4

load "~/work/pgr/conf/colorscheme.gnuplot"

FN_IN = system("echo $FN_IN")
FN_PLOT = system("echo $FN_PLOT")

set terminal pdfcairo enhanced size 4.5in, 3in
set output FN_PLOT

set xlabel "# of YouTube videos"
set ylabel "# of attrs / # of videos"
set grid lc rgb "#C0C0C0"
set border 3 lc rgb "#C0C0C0"

set xtics nomirror scale 0.5,0 tc rgb "#808080" ( \
   "0"      0, \
 "50K"  50000, \
"100K" 100000, \
"150K" 150000, \
"200K" 200000 \
)
set ytics nomirror scale 0.5,0 tc rgb "#808080" autofreq 0,0.25,1.25 format "%.2f"

set yrange [0.25:]

plot \
FN_IN u 1:($4/$1) w lines t "users", \
FN_IN u 1:($6/$1) w lines t "topics"
