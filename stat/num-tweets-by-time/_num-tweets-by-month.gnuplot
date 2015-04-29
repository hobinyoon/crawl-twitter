# Tested with gnuplot 4.6 patchlevel 4

FN_IN = system("echo $FN_IN")
FN_OUT = system("echo $FN_OUT")

set terminal pdfcairo enhanced size 4.5in, 3in
set output FN_OUT

set xlabel "Year-Month" font ",9"
set grid ytics lc rgb "#000000"
set border 3 lc rgb "#808080"

set xtics nomirror scale 0.5,0 font ",9" autofreq 0,1
# thousand separator (comma)
set decimal locale
set ytics nomirror scale 0.5,0 font ",9" format "%'g"
set yrange [0:]

set style fill solid 0.4 noborder
BOX_WIDTH=0.8
plot \
FN_IN u ($0+0.5):2:(BOX_WIDTH) w boxes not, \
FN_IN u 0:xticlabel(1) with points ps 0 not
