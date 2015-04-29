# Tested with gnuplot 4.6 patchlevel 4

load "~/work/pgr/conf/colorscheme.gnuplot"

FN_IN = system("echo $FN_IN")
FN_PLOT = system("echo $FN_PLOT")

set terminal pdfcairo enhanced size 4.5in, 3in
set output FN_PLOT

set xlabel "# of YouTube videos"
set ylabel "# of topics / # of videos"
set grid lc rgb "#C0C0C0"
set border 3 lc rgb "#C0C0C0"

set xtics nomirror scale 0.5,0 tc rgb "#808080" ( \
  "0"     0, \
"15K" 15000, \
"30K" 30000, \
"45K" 45000, \
"60K" 60000 \
)
set ytics nomirror scale 0.5,0 tc rgb "#808080" format "%.2f" autofreq 0,0.25,1.3
set key top right
set xrange [0:]
set yrange [:1.35]

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
