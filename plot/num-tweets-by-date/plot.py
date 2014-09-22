#! /usr/bin/python

import mysql.connector
import os
import sys
import subprocess


FN_DATA_BY_DATE = "num-tweets-by-date"
FN_DATA_BY_MONTH = "num-tweets-by-month"
FN_PLOT_BY_DATE = "num-tweets-by-date.pdf"
FN_PLOT_BY_MONTH = "num-tweets-by-month.pdf"
CUR_DIR = os.path.dirname(os.path.abspath(__file__))


def _ByDateGenData():
	conn = mysql.connector.connect(user="twitter", password="twitterpass", host="localhost", database="twitter")
	cursor = conn.cursor()
	query = ("select date(created_at) as d, count(*) as cnt from tweets group by d")
	cursor.execute(query)
	fo = open(FN_DATA_BY_DATE, "w")
	for (d1, cnt) in cursor:
		fo.write("%s %d\n" % (d1, cnt))
	fo.close()
	cursor.close()
	conn.close()


def _ByDatePlot():
	env = os.environ.copy()
	env["FN_IN"] = FN_DATA_BY_DATE
	env["FN_OUT"] = FN_PLOT_BY_DATE
	cmd = "gnuplot %s/_num-tweets-by-date.gnuplot" % CUR_DIR
	if subprocess.call(cmd, shell=True, env=env) != 0:
		raise RuntimeError("Error running cmd: %s" % cmd)
	print "  created %s" % FN_PLOT_BY_DATE


def _ByDate():
	_ByDateGenData()
	_ByDatePlot()


def _ByMonthGenData():
	conn = mysql.connector.connect(user="twitter", password="twitterpass", host="localhost", database="twitter")
	cursor = conn.cursor()
	query = ("select year(created_at) * 100 + month(created_at) as d, count(*) as cnt from tweets group by d")
	cursor.execute(query)
	fo = open(FN_DATA_BY_MONTH, "w")
	for (d1, cnt) in cursor:
		fo.write("%s %d\n" % (d1, cnt))
	fo.close()
	cursor.close()
	conn.close()


def _ByMonthPlot():
	env = os.environ.copy()
	env["FN_IN"] = FN_DATA_BY_MONTH
	env["FN_OUT"] = FN_PLOT_BY_MONTH

	cmd = "gnuplot %s/_num-tweets-by-month.gnuplot" % CUR_DIR
	if subprocess.call(cmd, shell=True, env=env) != 0:
		raise RuntimeError("Error running cmd: %s" % cmd)

	print "  created %s" % FN_PLOT_BY_MONTH


def _ByMonth():
	_ByMonthGenData()
	_ByMonthPlot()


def main(argv):
	_ByDate()
	_ByMonth()


if __name__ == "__main__":
	sys.exit(main(sys.argv))
