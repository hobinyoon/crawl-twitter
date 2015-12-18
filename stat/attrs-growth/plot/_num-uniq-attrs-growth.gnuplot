# Tested with gnuplot 4.6 patchlevel 6

FN_IN = system("echo $FN_IN")
FN_OUT = system("echo $FN_OUT")

# Get min and max values
set terminal unknown
plot \
FN_IN every 10 u ($1/1000):($4/$1) w lines, \
FN_IN every 10 u ($1/1000):($6/$1) w lines
X_MIN=GPVAL_DATA_X_MIN
X_MAX=GPVAL_DATA_X_MAX
Y_MIN=GPVAL_DATA_Y_MIN
Y_MAX=GPVAL_DATA_Y_MAX

set terminal pdfcairo enhanced size 2in, 1.75in
set output FN_OUT

set lmargin at screen 0.185
set rmargin at screen 0.971
set tmargin at screen 1.0
set bmargin at screen 0.183

set xlabel "# of YouTube videos (K)" offset 0,0.5
set ylabel "# of attrs / # of videos" offset 1.4,0
#set grid lc rgb "#808080"
set border 3 lc rgb "#808080"

set xtics nomirror scale 0.5,0 tc rgb "#808080" autofreq 0,100
set ytics nomirror scale 0.5,0 tc rgb "#808080" autofreq 0,0.5,1.25 format "%.1f"

X_MIN=0
Y_MIN=0.25
set xrange [X_MIN:X_MAX]
set yrange [Y_MIN:Y_MAX]

plot \
FN_IN every 10 u ($1/1000):($4/$1) w lines lc rgb "blue" t "User", \
FN_IN every 10 u ($1/1000):($6/$1) w lines lc rgb "red"  t "Topic"
