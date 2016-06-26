# Tested with gnuplot 4.6 patchlevel 6

load "~/work/gms/conf/colorscheme.gnuplot"

FN_IN = system("echo $FN_IN")
FN_OUT = system("echo $FN_OUT")

# Get min and max values
set terminal unknown
plot \
FN_IN every 8::400 u ($2/1000):($4/$2) w lines, \
FN_IN every 8::401 u ($2/1000):($4/$2) w lines, \
FN_IN every 8::402 u ($2/1000):($4/$2) w lines, \
FN_IN every 8::403 u ($2/1000):($4/$2) w lines, \
FN_IN every 8::404 u ($2/1000):($4/$2) w lines, \
FN_IN every 8::405 u ($2/1000):($4/$2) w lines, \
FN_IN every 8::406 u ($2/1000):($4/$2) w lines, \
FN_IN every 8::407 u ($2/1000):($4/$2) w lines
X_MIN=GPVAL_DATA_X_MIN
X_MAX=GPVAL_DATA_X_MAX
Y_MIN=GPVAL_DATA_Y_MIN
Y_MAX=GPVAL_DATA_Y_MAX

#set terminal pdfcairo enhanced size 3in, 2in
set terminal pdfcairo enhanced size 2in, 1.75in
set output FN_OUT

set rmargin at screen 0.99
set lmargin at screen 0.225
set tmargin at screen 1.0
set bmargin at screen 0.193

set xlabel "# of YouTube videos (K)" offset 0,0.4
set ylabel "# of topics / # of videos" offset 1.3,0
set border 3 lc rgb "#808080"

set xtics nomirror scale 0.5,0 tc rgb "#808080" autofreq 0, 25
set ytics nomirror scale 0.5,0 tc rgb "#808080" format "%.2f" autofreq 0,0.25,1.3
set key at 115,1.343

X_MIN=0
set xrange [X_MIN:X_MAX]
set yrange [Y_MIN:Y_MAX]

set pointsize 0.2

plot \
FN_IN every 8::400 u ($2/1000):($4/$2) w lines not, \
FN_IN every 8::401 u ($2/1000):($4/$2) w lines not, \
FN_IN every 8::402 u ($2/1000):($4/$2) w lines not, \
FN_IN every 8::403 u ($2/1000):($4/$2) w lines not, \
FN_IN every 8::404 u ($2/1000):($4/$2) w lines t "NewYork", \
FN_IN every 8::405 u ($2/1000):($4/$2) w lines t "SaoPaulo", \
FN_IN every 8::406 u ($2/1000):($4/$2) w lines t "Singapore", \
FN_IN every 8::407 u ($2/1000):($4/$2) w lines t "Warszawa"
