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
set key font ",9"

set pointsize 0.2

plot \
FN_IN every 700::700 u 2:($4/$2) w linespoints pt 7        lc rgb "#FF0000" t "Athens (topic)", \
FN_IN every 700::700 u 2:($3/$2) w linespoints pt 1 ps 0.3 lc rgb "#FF0000" t "Athens (user)", \
FN_IN every 700::701 u 2:($4/$2) w linespoints pt 7        lc rgb "#0000FF" t "California", \
FN_IN every 700::701 u 2:($3/$2) w linespoints pt 1 ps 0.3 lc rgb "#0000FF" not, \
FN_IN every 700::702 u 2:($4/$2) w linespoints pt 7        lc rgb "#00BB00" t "Ireland", \
FN_IN every 700::702 u 2:($3/$2) w linespoints pt 1 ps 0.3 lc rgb "#00BB00" not, \
FN_IN every 700::703 u 2:($4/$2) w linespoints pt 7        lc rgb "#BBBB00" t "Madrid", \
FN_IN every 700::703 u 2:($3/$2) w linespoints pt 1 ps 0.3 lc rgb "#BBBB00" not, \
FN_IN every 700::704 u 2:($4/$2) w linespoints pt 7        lc rgb "#DD00DD" t "New York", \
FN_IN every 700::704 u 2:($3/$2) w linespoints pt 1 ps 0.3 lc rgb "#DD00DD" not, \
FN_IN every 700::705 u 2:($4/$2) w linespoints pt 7        lc rgb "#00A0A0" t "SaoPaulo", \
FN_IN every 700::705 u 2:($3/$2) w linespoints pt 1 ps 0.3 lc rgb "#00A0A0" not, \
FN_IN every 700::706 u 2:($4/$2) w linespoints pt 7        lc rgb "#606060" t "Singapore", \
FN_IN every 700::706 u 2:($3/$2) w linespoints pt 1 ps 0.3 lc rgb "#606060" not
