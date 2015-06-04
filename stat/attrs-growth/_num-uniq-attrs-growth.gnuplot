# Tested with gnuplot 4.6 patchlevel 4

load "~/work/coco/conf/colorscheme.gnuplot"

FN_IN = system("echo $FN_IN")
FN_PLOT = system("echo $FN_PLOT")

set terminal pdfcairo enhanced size 3in, 2in
set output FN_PLOT

set rmargin at screen 0.97
set tmargin at screen 0.97

set xlabel "# of YouTube videos (K)" font ",14"
set ylabel "# of attrs / # of videos" offset 1.0,-0.4 font ",14"
#set grid lc rgb "#808080"
set border 3 lc rgb "#808080"

set xtics nomirror scale 0.5,0 tc rgb "#808080" ( \
"400" 400000, \
"300" 300000, \
"200" 200000, \
"100" 100000, \
  "0"      0 \
)
set ytics nomirror scale 0.5,0 tc rgb "#808080" autofreq 0,0.5,1.25 format "%.1f"

set key font ",14"

set yrange [0.25:]

plot \
FN_IN u 1:($4/$1) w lines t "User", \
FN_IN u 1:($6/$1) w lines t "Topic"
