# Tested with gnuplot 4.6 patchlevel 4

FN_IN = system("echo $FN_IN")
FN_OUT = system("echo $FN_OUT")
FN_DC_LOC = "../../../../grmts/conf/youtube-dc-coord"
FN_WORLD_MAP = "../../../worldmap/world_110m.txt"

set terminal pdfcairo enhanced size 2in, 1.75in
set output FN_OUT

set lmargin at screen 0
set rmargin at screen 1
set tmargin at screen 1
set bmargin at screen 0

set noborder
set noxtics
set noytics

set xrange[-140:140]
set yrange[-80:80]

#set label 1 "Number of requests in 0.5{/Symbol \260}-longitude/latitude-wide unit area" at -60,20 center font ",9" front
#set object 2 rect from -110,-88 to 110,-62 fs empty border lc rgb "#D0D0D0" front

set label "California" at -122.117012,  37.426207 center offset 1.85,-0.7 tc rgb "black" front
set label "Ireland"    at   -6.265152,  53.348194 center offset    0, 0.8 tc rgb "black" front
set label "Singapore"  at  103.789807,   1.295437 center offset -0.5,-0.7 tc rgb "black" front
set label "Warszawa"   at   21.013099,  52.233813 left   offset  0.4, 0.5 tc rgb "black" front
set label "NewYork"    at  -73.984973,  40.757807 center offset    0, 0.8 tc rgb "black" front
set label "Milan"      at    9.202015,  45.490065 left   offset  0.4,-0.5 tc rgb "black" front
set label "SaoPaulo"   at  -46.661744, -23.556599 center offset    0,-0.8 tc rgb "black" front
set label "Madrid"     at   -3.703812,  40.416759 center offset    0,-0.8 tc rgb "black" front

plot \
FN_WORLD_MAP with filledcurves fs transparent solid 0.50 noborder lc rgb "#C0C0C0" not, \
FN_IN     u 1:2:4 with circles fs transparent solid 0.35 noborder lc rgb "#FF0000" lw 0.4 not, \
FN_IN     u 1:($2-8.0):5 with labels tc rgb "#000000" not, \
FN_DC_LOC u 3:2:(2.0) with circles lt 3 lc rgb "black" fs solid 1.0 noborder not
