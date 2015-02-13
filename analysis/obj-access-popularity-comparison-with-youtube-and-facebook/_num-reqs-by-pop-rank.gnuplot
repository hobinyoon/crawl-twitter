# Tested with gnuplot 4.6 patchlevel 4

FN_IN = system("echo $FN_IN")
FN_OUT = system("echo $FN_OUT")
FN_FB = system("echo $FN_FB")

load "~/work/pgr/conf/colorscheme.gnuplot"

set terminal pdfcairo enhanced size 4.5in, 3in
set output FN_OUT
set xlabel "Popularity ranking"
set ylabel "Number of access" offset -1,0
set border (1 + 2 + 8) lc rgb "#C0C0C0"
set grid xtics back lt 0 lc rgb "#808080"

set xtics nomirror scale 1.0,0.5 tc rgb "#202020"
set mxtics 10

# times symbol 264
set ytics nomirror scale 1.0,0.5 offset 0,0.2 tc rgb "#0000FF" (\
		"2{/Symbol \264}10^3" 2000, \
		"10^3"                1000, \
		""                     900, \
		""                     800, \
		""                     700, \
		""                     600, \
		""                     500, \
		"4{/Symbol \264}10^2"  400, \
		""                     300, \
		"2{/Symbol \264}10^2"  200, \
		""                     100 \
)

set y2tics nomirror scale 1.0,0.5 right offset 5,0.2 tc rgb "#FF0000" ( \
		"10^9" 1, \
		"10^8" 0.75, \
		""     0.5 + (0.25 + log10(2)),  \
		"10^7" 0.5,  \
		""                    0.25 + (0.25 * log10(9)), \
		""                    0.25 + (0.25 * log10(8)), \
		""                    0.25 + (0.25 * log10(7)), \
		""                    0.25 + (0.25 * log10(6)), \
		""                    0.25 + (0.25 * log10(5)), \
		"4{/Symbol \264}10^6" 0.25 + (0.25 * log10(4)), \
		""                    0.25 + (0.25 * log10(3)), \
		"2{/Symbol \264}10^6" 0.25 + (0.25 * log10(2)), \
		"10^6"                0.25, \
		""                    0    + (0.25 * log10(9)), \
		""                    0    + (0.25 * log10(8)), \
		"10^5"                0 \
)

set logscale xy
set format xy "10^{%L}"
set xrange [:1000]
set yrange [124:2357]

plot \
FN_FB u 1:2 axes x2y2 w lines t "Huang 2014", \
FN_IN u 0:1 w lines t "Our dataset"
