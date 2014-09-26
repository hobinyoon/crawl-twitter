# Tested with gnuplot 4.6 patchlevel 4

FN_IN = system("echo $FN_IN")
FN_OUT = system("echo $FN_OUT")
CB_RANGE_MAX = system("echo $CB_RANGE_MAX")

set terminal pngcairo enhanced size 1600,800
set output FN_OUT

unset key
set grid cbtics lt 1 lc rgb "#FF0000"

# Color runs from white to green
set palette rgbformula -7,2,-7
set cbrange [0:CB_RANGE_MAX]
set cblabel "# of tweets"
set cbtics scale 0.1 font ",7" autofreq 0, 1
unset cbtics

set view map
splot FN_IN matrix with image
