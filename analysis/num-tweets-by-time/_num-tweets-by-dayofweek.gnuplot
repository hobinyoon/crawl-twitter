# Tested with gnuplot 4.6 patchlevel 4

FN_IN = system("echo $FN_IN")
FN_OUT = system("echo $FN_OUT")

set terminal pdfcairo enhanced size 4.5in, 3in
set output FN_OUT

set lmargin at screen 0.1
set grid lc rgb "#808080"
set border 3 lc rgb "#808080"

set xtics nomirror scale 0.5,0 font ",9" ( \
	"Sun" 1, \
	"Mon" 2, \
	"Tue" 3, \
	"Wed" 4, \
	"Thr" 5, \
	"Fri" 6, \
	"Sat" 7 \
	)

# thousand separator (comma)
set decimal locale
set ytics nomirror scale 0.5,0 font ",9" format "%'g"
set yrange [0:]

set style fill solid 0.4 noborder
BOX_WIDTH=0.8
plot \
FN_IN u 1:2:(BOX_WIDTH) w boxes not
