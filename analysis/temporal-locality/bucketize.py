#!/usr/bin/env python

import datetime
import os
import sys
import Conf
import Util


_dt_cnt = {}

def ReadTweetsBucketizeDate():
	with open(Conf.fn_tweet) as fo:
		for line in fo:
			if len(line) == 0:
				continue
			if line[0:1] == "#":
				continue
			#Util.Trace(line)
			# May or may not want to filter by exact word
			line = line.strip()
			t = line.split("\t")
			if len(t) != 11:
				raise RuntimeError("Unexpected format [%s]" % (line))
			datetime0 = t[2]
			#Util.Trace(datetime0)
			dt_ymd = datetime0[0:10]

			dt = datetime.datetime.strptime(datetime0[0:10], "%Y-%m-%d")
			#print dt.toordinal()
			dates_rounded = datetime.date.fromordinal(dt.toordinal() / Conf.bucket_size * Conf.bucket_size)

			if dates_rounded in _dt_cnt:
				_dt_cnt[dates_rounded] += 1
			else:
				_dt_cnt[dates_rounded] = 1


def Write():
	fn = Conf.fn_output
	with open(fn, "w") as fo:
		fo.write("# datetime cnt\n")
		for k, v in sorted(_dt_cnt.items()):
			fo.write("%s %d\n" % (k, v))
	print "Created file %s %d" % (fn, os.path.getsize(fn))


def main(argv):
	Conf.ParseOpts(argv)
	ReadTweetsBucketizeDate()
	Write()


if __name__ == "__main__":
  sys.exit(main(sys.argv))
