#! /usr/bin/python

import mysql.connector
import os
import sys
import subprocess

DN_STAT="/mnt/mdc-data/pgr/twitter/stat"

FN_DATA_BY_DATE =      DN_STAT + "/num-tweets-by-date"
FN_DATA_BY_MONTH =     DN_STAT + "/num-tweets-by-month"
FN_DATA_BY_DAYOFWEEK = DN_STAT + "/num-tweets-by-dayofweek"
FN_DATA_BY_HOUR =      DN_STAT + "/num-tweets-by-hour"

FN_PLOT_BY_DATE =      DN_STAT + "/num-tweets-by-date.pdf"
FN_PLOT_BY_MONTH =     DN_STAT + "/num-tweets-by-month.pdf"
FN_PLOT_BY_DAYOFWEEK = DN_STAT + "/num-tweets-by-dayofweek.pdf"
FN_PLOT_BY_HOUR =      DN_STAT + "/num-tweets-by-hour.pdf"

CUR_DIR = os.path.dirname(os.path.abspath(__file__))

DB_USER="twitter"
DB_PW="twitterpass"
DB_HOST="localhost"
DB_NAME="twitter2"


def _ByDateGenData():
	conn = mysql.connector.connect(user=DB_USER, password=DB_PW, host=DB_HOST, database=DB_NAME)
	cursor = conn.cursor()
	query = ("SELECT DATE(created_at) AS d, COUNT(*) AS cnt FROM tweets GROUP BY d")
	cursor.execute(query)
	fo = open(FN_DATA_BY_DATE, "w")
	for (d, cnt) in cursor:
		fo.write("%s %d\n" % (d, cnt))
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
	print "created %s" % FN_PLOT_BY_DATE


def _ByDate():
	_ByDateGenData()
	_ByDatePlot()


def _ByMonthGenData():
	conn = mysql.connector.connect(user=DB_USER, password=DB_PW, host=DB_HOST, database=DB_NAME)
	cursor = conn.cursor()
	query = ("SELECT DATE_FORMAT(created_at, '%y-%m') AS d, COUNT(*) AS cnt FROM tweets GROUP BY d")
	cursor.execute(query)
	fo = open(FN_DATA_BY_MONTH, "w")
	for (d, cnt) in cursor:
		fo.write("%s %d\n" % (d, cnt))
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

	print "created %s" % FN_PLOT_BY_MONTH


def _ByMonth():
	_ByMonthGenData()
	_ByMonthPlot()


def _ByDayofweekGenData():
	conn = mysql.connector.connect(user=DB_USER, password=DB_PW, host=DB_HOST, database=DB_NAME)
	cursor = conn.cursor()
	query = ("SELECT DAYOFWEEK(created_at) AS d, COUNT(*) AS cnt FROM tweets GROUP BY d")
	cursor.execute(query)
	fo = open(FN_DATA_BY_DAYOFWEEK, "w")
	for (d, cnt) in cursor:
		fo.write("%s %d\n" % (d, cnt))
	fo.close()
	cursor.close()
	conn.close()


def _ByDayofweekPlot():
	env = os.environ.copy()
	env["FN_IN"] = FN_DATA_BY_DAYOFWEEK
	env["FN_OUT"] = FN_PLOT_BY_DAYOFWEEK

	cmd = "gnuplot %s/_num-tweets-by-dayofweek.gnuplot" % CUR_DIR
	if subprocess.call(cmd, shell=True, env=env) != 0:
		raise RuntimeError("Error running cmd: %s" % cmd)

	print "created %s" % FN_PLOT_BY_DAYOFWEEK


def _ByDayofweek():
	_ByDayofweekGenData()
	_ByDayofweekPlot()


def _ByHourGenData():
	conn = mysql.connector.connect(user=DB_USER, password=DB_PW, host=DB_HOST, database=DB_NAME)
	cursor = conn.cursor()
	query = ("SELECT DATE_FORMAT(created_at, '%H') AS d, COUNT(*) AS cnt FROM tweets GROUP BY d")
	cursor.execute(query)
	fo = open(FN_DATA_BY_HOUR, "w")
	for (d, cnt) in cursor:
		fo.write("%s %d\n" % (d, cnt))
	fo.close()
	cursor.close()
	conn.close()


def _ByHourPlot():
	env = os.environ.copy()
	env["FN_IN"] = FN_DATA_BY_HOUR
	env["FN_OUT"] = FN_PLOT_BY_HOUR

	cmd = "gnuplot %s/_num-tweets-by-hour.gnuplot" % CUR_DIR
	if subprocess.call(cmd, shell=True, env=env) != 0:
		raise RuntimeError("Error running cmd: %s" % cmd)

	print "created %s" % FN_PLOT_BY_HOUR


def _ByHour():
	_ByHourGenData()
	_ByHourPlot()


def main(argv):
	#_ByDate()

	_ByMonth()
	_ByDayofweek()
	_ByHour()


if __name__ == "__main__":
	sys.exit(main(sys.argv))
