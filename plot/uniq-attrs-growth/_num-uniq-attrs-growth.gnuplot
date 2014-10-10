# Tested with gnuplot 4.6 patchlevel 4

load "~/work/pgr/conf/colorscheme.gnuplot"

FN_IN = system("echo $FN_IN")
FN_PLOT = system("echo $FN_PLOT")

set terminal pdfcairo enhanced size 4.5in, 3in
set output FN_PLOT

set xlabel "# of uniq YouTube videos" font ",9"
set grid lc rgb "#808080"
set border 3 lc rgb "#808080"

# thousand separator (comma)
set decimal locale
set xtics nomirror scale 0.5,0 font ",9" format "%'g"
set ytics nomirror scale 0.5,0 font ",9" format "%.1f"

set pointsize 0.2

plot \
FN_IN every 1000::1000 u 1:($4/$1) w linespoints pt 7 t "topics", \
FN_IN every 1000::1000 u 1:($3/$1) w linespoints pt 7 t "users"
