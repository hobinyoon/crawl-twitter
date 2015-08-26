# Tested with gnuplot 4.6 patchlevel 4

FN_IN = system("echo $FN_IN")
FN_OUT = system("echo $FN_OUT")
FN_WORLD_MAP="~/work/crawl-twitter/worldmap/world_110m.txt"

set terminal pdfcairo enhanced size 2in, 1.5in
set output FN_OUT

set lmargin at screen 0.0
set rmargin at screen 0.992
set tmargin at screen 1.0
set bmargin at screen 0.0

set grid lc rgb "#C0C0C0"
set noborder
set noxtics
set noytics
#set xtics nomirror scale 0,0 format "%.0f{/Symbol \260}" tc rgb "#808080"
#set ytics nomirror scale 0,0 format "%.0f{/Symbol \260}" tc rgb "#808080"

X_MIN=-170
X_MAX=180
set xrange[X_MIN:X_MAX]
set yrange[-80:85]

CIRCLE_SIZE(x)=sqrt(x)*6.0

# Legend
X_MIDDLE = (X_MIN+X_MAX)/2.0
CS="1 2 5 10 15"
SPACING=45
do for [i=0:4] {
	X=X_MIDDLE-(4/2*SPACING)+(i*SPACING)
	Y=-45
	set obj circle at X,Y size CIRCLE_SIZE(word(CS, i+1)) \
		fs transparent solid 0.35 noborder fc rgb "#FF0000" front

	Y=-65
	set label word(CS, i+1) at X,Y center tc rgb "#808080" front
}

plot \
FN_IN u 1:2:(CIRCLE_SIZE($3)) with circles fs transparent solid 0.35 noborder lc rgb "#FF0000" not, \
FN_WORLD_MAP w filledcurves fs transparent solid 0.25 noborder lc rgb "#C0C0C0" not
