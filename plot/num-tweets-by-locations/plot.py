#! /usr/bin/python

import math
import mysql.connector
import os
import sys
import subprocess
import time

DN_STAT="/mnt/mdc-data/pgr/twitter/stat"

FN_DATA_BY_LOC = DN_STAT + "/num-tweets-by-loc"
FN_PLOT_BY_LOC = DN_STAT + "/num-tweets-by-loc.pdf"
FN_DATA_BY_LOC_BY_HOUR = DN_STAT + "/num-tweets-by-loc-by-hour"

CUR_DIR = os.path.dirname(os.path.abspath(__file__))

DB_USER="twitter"
DB_PW="twitterpass"
DB_HOST="localhost"
DB_NAME="twitter2"


def _CircleSize(n):
	return math.sqrt(n)/75.0


def _CircleSizeByHour(n):
	return math.sqrt(n)/20.0


def _ByLocGenData():
	tmr = time.time()
	tmr1 = time.time()
	print "Generating data ..."
	conn = mysql.connector.connect(user=DB_USER, password=DB_PW, host=DB_HOST, database=DB_NAME)
	cursor = conn.cursor()
	#query = ("SELECT ROUND(geo_longi * 2)/2 AS longi, ROUND(geo_lati*2)/2 AS lati, COUNT(*) AS cnt "
	query = ("SELECT ROUND(geo_longi) AS longi, ROUND(geo_lati) AS lati, COUNT(*) AS cnt "
			"FROM tweets GROUP BY longi, lati")
	cursor.execute(query)
	print "  execute query: %0.3f sec" % (time.time() - tmr1)

	fo = open(FN_DATA_BY_LOC, "w")
	for (longi, lati, cnt) in cursor:
		fo.write("%f %f %f\n" % (longi, lati, _CircleSize(cnt)))
	p = 0
	for i in [
			1,
			10,
			100,
			1000,
			10000,
			20000]:
		fo.write("%f %f %f %d\n"
				% (-50 + 20*p, -75, _CircleSize(i), i))
		p += 1
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


def _ByLocByHourGenData():
	tmr = time.time()
	tmr1 = time.time()
	print "Generating data ..."
	conn = mysql.connector.connect(user=DB_USER, password=DB_PW, host=DB_HOST, database=DB_NAME)
	cursor = conn.cursor()
	query = ("SELECT DATE_FORMAT(created_at, '%H') AS h, "
			"ROUND(geo_longi * 2)/2 AS longi, ROUND(geo_lati*2)/2 AS lati, "
			"COUNT(*) AS cnt "
			"FROM tweets GROUP BY h, longi, lati")
	cursor.execute(query)
	print "  execute query: %0.3f sec" % (time.time() - tmr1)

	fn_fo = {}
	for (h, longi, lati, cnt) in cursor:
		fn = FN_DATA_BY_LOC_BY_HOUR + "-" + h
		fo = None
		if fn in fn_fo:
			fo = fn_fo[fn]
		else:
			fo = open(fn, "w")
			fn_fo[fn] = fo
		fo.write("%.1f %.1f %.2f\n" % (longi, lati, _CircleSizeByHour(cnt)))

	for fn, fo in fn_fo.items():
		p = 0
		for i in [
				1,
				10,
				100,
				1000,
				1500]:
			fo.write("%.1f %.1f %.2f %d\n"
					% (-40 + 20*p, -75, _CircleSizeByHour(i), i))
			p += 1
		fo.close()

	cursor.close()
	conn.close()
	print "  %0.3f sec" % (time.time() - tmr)


def _ByLocByHourPlot():
	tmr = time.time()
	print "Plotting ..."
	env = os.environ.copy()
	env["FN_IN"] = FN_DATA_BY_LOC_BY_HOUR
	cmd = "gnuplot %s/_num-tweets-by-loc-by-hour.gnuplot" % CUR_DIR
	if subprocess.call(cmd, shell=True, env=env) != 0:
		raise RuntimeError("Error running cmd: %s" % cmd)

	# Not very satisfactory.
	# cmd = "convert -layers Optimize -delay 200"
	# for i in range(24):
	# 	cmd += (" %s-%02d.pdf" % (FN_DATA_BY_LOC_BY_HOUR, i))
	# cmd += (" %s.gif" % FN_DATA_BY_LOC_BY_HOUR)
	# if subprocess.call(cmd, shell=True) != 0:
	# 	raise RuntimeError("Error running cmd: %s" % cmd)

	print "  %0.3f sec" % (time.time() - tmr)


def _ByLocByHour():
	#_ByLocByHourGenData()
	_ByLocByHourPlot()


def main(argv):
	#_ByLoc()
	_ByLocByHour()


if __name__ == "__main__":
	sys.exit(main(sys.argv))
