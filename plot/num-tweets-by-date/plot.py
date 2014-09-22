#! /usr/bin/python

import mysql.connector
import os
import sys
import subprocess


FN_DATA = "num-tweets-by-date"
FN_PLOT = "num-tweets-by-date.pdf"
CUR_DIR = os.path.dirname(os.path.abspath(__file__))


def _GenNumByDate():
	conn = mysql.connector.connect(user="twitter", password="twitterpass", host="localhost", database="twitter")
	cursor = conn.cursor()
	query = ("select date(created_at) as d, count(*) as cnt from tweets group by d")
	cursor.execute(query)
	fo = open(FN_DATA, "w")
	for (d1, cnt) in cursor:
		fo.write("%s %d\n" % (d1, cnt))
	fo.close()
	cursor.close()
	conn.close()


def _Plot():
	env = os.environ.copy()
	env["FN_IN"] = FN_DATA
	env["FN_OUT"] = FN_PLOT

	cmd = "gnuplot %s/_num-tweets-by-date.gnuplot" % CUR_DIR
	if subprocess.call(cmd, shell=True, env=env) != 0:
		raise RuntimeError("Error running cmd: %s" % cmd)

	print "  created %s" % FN_PLOT


def main(argv):
	_GenNumByDate()
	_Plot()


if __name__ == "__main__":
	sys.exit(main(sys.argv))
