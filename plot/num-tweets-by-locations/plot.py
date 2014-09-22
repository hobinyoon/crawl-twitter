#! /usr/bin/python

import mysql.connector
import os
import sys
import subprocess
import time


FN_DATA_BY_LOC = "num-tweets-by-loc"
FN_PLOT_BY_LOC = "num-tweets-by-loc.png"
CUR_DIR = os.path.dirname(os.path.abspath(__file__))


def _ByLocGenData():
	tmr = time.time()
	print "Generating data ..."
	conn = mysql.connector.connect(user="twitter", password="twitterpass", host="localhost", database="twitter")
	cursor = conn.cursor()
	query = ("select geo_longi, geo_lati from tweets")
	cursor.execute(query)
	fo = open(FN_DATA_BY_LOC, "w")
	for (longi, lati) in cursor:
		fo.write("%f %f\n" % (longi, lati))
	fo.close()
	cursor.close()
	conn.close()
	print "  %0.3f sec" % (time.time() - tmr)


def _ByLocPlot():
	tmr = time.time()
	print "Plotting ..."
	env = os.environ.copy()
	env["FN_IN"] = FN_DATA_BY_LOC
	env["FN_OUT"] = FN_PLOT_BY_LOC
	cmd = "gnuplot %s/_num-tweets-by-loc.gnuplot" % CUR_DIR
	if subprocess.call(cmd, shell=True, env=env) != 0:
		raise RuntimeError("Error running cmd: %s" % cmd)
	print "  created %s" % FN_PLOT_BY_LOC
	print "  %0.3f sec" % (time.time() - tmr)


def _ByLoc():
	_ByLocGenData()
	_ByLocPlot()


def main(argv):
	_ByLoc()


if __name__ == "__main__":
	sys.exit(main(sys.argv))
