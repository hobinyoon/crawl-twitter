# Tested with gnuplot 4.6 patchlevel 4

FN_IN = "rogers-1000-followers"
FN_OUT = "rogers-1000-followers.pdf"

set terminal pdfcairo enhanced size 4.5in, 3in

set output FN_OUT

plot \
FN_IN u 1:(0) w points not
