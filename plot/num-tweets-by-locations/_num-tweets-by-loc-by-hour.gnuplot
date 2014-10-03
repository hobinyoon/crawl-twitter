# Tested with gnuplot 4.6 patchlevel 4

FN_IN = system("echo $FN_IN")

set terminal pdfcairo enhanced size 4.5in,3in

set grid lc rgb "#000000"
set border 3 lc rgb "#808080"
set xtics nomirror scale 0.5,0 font ",9" format "%.0f{/Symbol \260}" autofreq -180,60
set ytics nomirror scale 0.5,0 font ",9" format "%.0f{/Symbol \260}" autofreq -90,30

set xrange[-180:180]
set yrange[-90:90]

set label 1 "Number of requests in 1{/Symbol \260}-longitude/latitude-wide unit area" at 0,-65 center font ",9" front
set object 2 rect from -110,-88 to 110,-62 fs empty border lc rgb "#D0D0D0" front

do for [i=0:23] {
	set title sprintf("UTC %02dH", i) font ",9"
	fn_in_ = sprintf("%s-%02d", FN_IN, i)
	fn_out = fn_in_ . ".pdf"

	set output fn_out

	plot \
	'world_110m.txt' with filledcurves ls 1 lc rgb "#F0F0F0" not, \
	fn_in_ u 1:2:3 with points pt 6 ps variable lc rgb "#FF0000" lw 0.4 not, \
	fn_in_ u 1:($2-10):4 with labels tc rgb "#000000" font ",7" not
}
