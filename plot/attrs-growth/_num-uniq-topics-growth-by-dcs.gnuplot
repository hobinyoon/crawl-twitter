# Tested with gnuplot 4.6 patchlevel 4

load "~/work/pgr/conf/colorscheme.gnuplot"

FN_IN = system("echo $FN_IN")
FN_PLOT = system("echo $FN_PLOT")

set terminal pdfcairo enhanced size 4.5in, 3in
set output FN_PLOT

set xlabel "# of YouTube videos"
set ylabel "# of topics / # of videos"
set grid lc rgb "#808080"
set border 3 lc rgb "#808080"

set xtics nomirror scale 0.5,0 ( \
"10K" 10000, \
"20K" 20000, \
"30K" 30000, \
"40K" 40000, \
"50K" 50000 \
)
set ytics nomirror scale 0.5,0 format "%.1f"
set key top right font ",10" spacing 0.9
#set yrange [0:]

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

# FN_IN every 8::800 u 2:($4/$2) w linespoints pt 7 t "California", \
# FN_IN every 8::801 u 2:($4/$2) w linespoints pt 7 t "Ireland", \
# FN_IN every 8::802 u 2:($4/$2) w linespoints pt 7 t "Madrid", \
# FN_IN every 8::803 u 2:($4/$2) w linespoints pt 7 t "Milan", \
# FN_IN every 8::804 u 2:($4/$2) w linespoints pt 7 t "NewYork", \
# FN_IN every 8::805 u 2:($4/$2) w linespoints pt 7 t "SaoPaulo", \
# FN_IN every 8::806 u 2:($4/$2) w linespoints pt 7 t "Singapore", \
# FN_IN every 8::807 u 2:($4/$2) w linespoints pt 7 t "Warszawa"
