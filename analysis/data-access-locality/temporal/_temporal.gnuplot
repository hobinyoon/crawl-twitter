# Tested with gnuplot 4.6 patchlevel 4

FN_IN = system("echo $FN_IN")
FN_OUT = system("echo $FN_OUT")

set terminal pdfcairo enhanced size 2in, 1.5in
set output FN_OUT

set lmargin at screen 0.248
set rmargin at screen 0.992
set tmargin at screen 0.97
set bmargin at screen 0.172

set ylabel "Number of accesses\nper week" offset 2.4,0

set xdata time
set timefmt "%Y-%m-%d"

set border 3 lc rgb "#808080" front
set xtics nomirror scale 0.5,0 autofreq 0,(365/12.0*2)*24*3600 rotate by -30 offset 0,-0.2
set ytics nomirror scale 0.5,0 autofreq 0,25
#set format x "'%y"
set format x "%b"

set xrange["2014-01-01":"2014-12-31"]
# This shows the annual pattern, but thought it might be a distractin.
#set xrange[:"2015-02-11"]

set yrange[0:]

plot FN_IN u 1:2 w linespoints pt 7 pointsize 0.3 lc rgb "#FF0000" not

#set boxwidth (7*24*3600)*(0.9)
#set style fill transparent solid 0.6 noborder
#plot FN_IN u 1:2 w boxes not
