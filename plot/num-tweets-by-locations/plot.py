#! /usr/bin/python

import math
import mysql.connector
import os
import sys
import subprocess
import time


FN_DATA_BY_LOC = "num-tweets-by-loc"
FN_PLOT_BY_LOC = "num-tweets-by-loc.png"
CUR_DIR = os.path.dirname(os.path.abspath(__file__))

DB_USER="twitter"
DB_PW="twitterpass"
DB_HOST="localhost"
DB_NAME="twitter2"


def _ByLocGenData():
	tmr = time.time()
	tmr1 = time.time()
	print "Generating data ..."
	conn = mysql.connector.connect(user=DB_USER, password=DB_PW, host=DB_HOST, database=DB_NAME)
	cursor = conn.cursor()
	#query = ("select geo_longi, geo_lati from tweets")
	query = ("select (round(geo_longi/5, 1)*5) as longi, (round(geo_lati/5, 1)*5) as lati, count(*) as cnt from tweets group by longi, lati order by longi, lati")
	cursor.execute(query)
	print "  execute query: %0.3f sec" % (time.time() - tmr1)

	fo = open(FN_DATA_BY_LOC, "w")
	for (longi, lati, cnt) in cursor:
		fo.write("%f %f %d\n" % (longi, lati, math.sqrt(cnt)))
	cursor.close()
	conn.close()
	fo.close()
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
