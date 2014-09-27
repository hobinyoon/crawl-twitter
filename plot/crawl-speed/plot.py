#! /usr/bin/python

import mysql.connector
import os
import sys
import subprocess
import time


FN_USERS_CRAWLED_PER_HOUR = "users-crawled-per-hour"
FN_PLOT_USERS_CRAWLED_PER_HOUR = FN_USERS_CRAWLED_PER_HOUR + ".pdf"
FN_USERS_CRAWLED_PER_DAY = "users-crawled-per-day"
FN_PLOT_USERS_CRAWLED_PER_DAY = FN_USERS_CRAWLED_PER_DAY + ".pdf"
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
	query = ("select date_format(crawled_at, '%Y%m%d%H') as d, count(*) as cnt from users where status='C' group by d")
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
	cmd = "gnuplot %s/_users-crawled-per-hour.gnuplot" % CUR_DIR
	if subprocess.call(cmd, shell=True, env=env) != 0:
		raise RuntimeError("Error running cmd: %s" % cmd)
	print "  created %s" % FN_PLOT_USERS_CRAWLED_PER_HOUR
	print "  %0.3f sec" % (time.time() - tmr)


def _PerDayGetData():
	print "Getting data ..."
	tmr = time.time()
	conn = mysql.connector.connect(user=DB_USER, password=DB_PW, host=DB_HOST, database=DB_NAME)
	cursor = conn.cursor()
	query = ("select date_format(crawled_at, '%Y%m%d') as d, count(*) as cnt from users where status='C' group by d")
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
	cmd = "gnuplot %s/_users-crawled-per-day.gnuplot" % CUR_DIR
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
