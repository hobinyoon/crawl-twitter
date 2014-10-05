#! /usr/bin/python

import mysql.connector
import os
import sys
import subprocess
import time

DN_STAT="/mnt/mdc-data/pgr/twitter/stat"

FN_USERS_CRAWLED_PER_HOUR      = DN_STAT + "/users-crawled-by-hour"
FN_PLOT_USERS_CRAWLED_PER_HOUR = FN_USERS_CRAWLED_PER_HOUR + ".pdf"
FN_USERS_CRAWLED_PER_DAY       = DN_STAT + "/users-crawled-by-day"
FN_PLOT_USERS_CRAWLED_PER_DAY  = FN_USERS_CRAWLED_PER_DAY + ".pdf"

CUR_DIR = os.path.dirname(os.path.abspath(__file__))

DB_USER="twitter"
DB_PW="twitterpass"
DB_HOST="localhost"
DB_NAME="twitter2"


def _PerHourGetData():
	print "Getting data ..."
	tmr = time.time()
	conn = mysql.connector.connect(user=DB_USER, password=DB_PW, host=DB_HOST, database=DB_NAME)
	cursor = conn.cursor()
	query = ("SELECT DATE_FORMAT(crawled_at, '%H') AS d, COUNT(*) AS cnt FROM users WHERE status='C' GROUP BY d")
	cursor.execute(query)
	fo = open(FN_USERS_CRAWLED_PER_HOUR, "w")
	for (d, cnt) in cursor:
		fo.write("%s %d\n" % (d, cnt))
	fo.close()
	cursor.close()
	conn.close()
	print "  %0.3f sec" % (time.time() - tmr)


def _PerHourPlot():
	print "Plotting ..."
	tmr = time.time()
	env = os.environ.copy()
	env["FN_IN"] = FN_USERS_CRAWLED_PER_HOUR
	env["FN_OUT"] = FN_PLOT_USERS_CRAWLED_PER_HOUR
	cmd = "gnuplot %s/_users-crawled-by-hour.gnuplot" % CUR_DIR
	if subprocess.call(cmd, shell=True, env=env) != 0:
		raise RuntimeError("Error running cmd: %s" % cmd)
	print "  created %s" % FN_PLOT_USERS_CRAWLED_PER_HOUR
	print "  %0.3f sec" % (time.time() - tmr)


def _PerDayGetData():
	print "Getting data ..."
	tmr = time.time()
	conn = mysql.connector.connect(user=DB_USER, password=DB_PW, host=DB_HOST, database=DB_NAME)
	cursor = conn.cursor()
	query = ("SELECT DATE_FORMAT(crawled_at, '%Y%m%d') AS d, COUNT(*) AS cnt FROM users WHERE status='C' GROUP BY d")
	cursor.execute(query)
	fo = open(FN_USERS_CRAWLED_PER_DAY, "w")
	for (d, cnt) in cursor:
		fo.write("%s %d\n" % (d, cnt))
	fo.close()
	cursor.close()
	conn.close()
	print "  %0.3f sec" % (time.time() - tmr)


def _PerDayPlot():
	print "Plotting ..."
	tmr = time.time()
	env = os.environ.copy()
	env["FN_IN"] = FN_USERS_CRAWLED_PER_DAY
	env["FN_OUT"] = FN_PLOT_USERS_CRAWLED_PER_DAY
	cmd = "gnuplot %s/_users-crawled-by-day.gnuplot" % CUR_DIR
	if subprocess.call(cmd, shell=True, env=env) != 0:
		raise RuntimeError("Error running cmd: %s" % cmd)
	print "  created %s" % FN_PLOT_USERS_CRAWLED_PER_DAY
	print "  %0.3f sec" % (time.time() - tmr)


def PerHour():
	_PerHourGetData()
	_PerHourPlot()


def PerDay():
	_PerDayGetData()
	_PerDayPlot()


def main(argv):
	PerHour()
	PerDay()


if __name__ == "__main__":
	sys.exit(main(sys.argv))
