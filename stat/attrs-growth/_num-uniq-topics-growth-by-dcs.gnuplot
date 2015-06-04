# Tested with gnuplot 4.6 patchlevel 4

load "~/work/coco/conf/colorscheme.gnuplot"

FN_IN = system("echo $FN_IN")
FN_PLOT = system("echo $FN_PLOT")

set terminal pdfcairo enhanced size 3in, 2in
set output FN_PLOT

set rmargin at screen 0.97
set tmargin at screen 0.97

set xlabel "# of YouTube videos (K)" font ",14"
set ylabel "# of topics / # of videos" offset 1.0,-0.4 font ",14"
#set grid lc rgb "#808080"
set border 3 lc rgb "#808080"

set xtics nomirror scale 0.5,0 tc rgb "#808080" ( \
"100" 100000, \
 "75"  75000, \
 "50"  50000, \
 "25"  25000, \
  "0"      0 \
)
set ytics nomirror scale 0.5,0 tc rgb "#808080" format "%.2f" autofreq 0,0.25,1.3
set key top right maxrows 4
set xrange [0:]
set yrange [:1.40]

set pointsize 0.2

plot \
FN_IN every 8::400 u 2:($4/$2) w lines t "California", \
FN_IN every 8::401 u 2:($4/$2) w lines t "Ireland", \
FN_IN every 8::402 u 2:($4/$2) w lines t "Madrid", \
FN_IN every 8::403 u 2:($4/$2) w lines t "Milan", \
FN_IN every 8::404 u 2:($4/$2) w lines t "NewYork", \
FN_IN every 8::405 u 2:($4/$2) w lines t "SaoPaulo", \
FN_IN every 8::406 u 2:($4/$2) w lines t "Singapore", \
FN_IN every 8::407 u 2:($4/$2) w lines t "Warszawa"
