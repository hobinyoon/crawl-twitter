#!/usr/bin/env python

import datetime
import os
import sys
import Conf
import Util


_dt_cnt = {}
_dt_min = None
_dt_max = None

def ReadTweetsBucketizeDate():
	global _dt_min
	global _dt_max
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
			if _dt_min is None:
				_dt_min = dates_rounded
			else:
				_dt_min = min(_dt_min, dates_rounded)
			if _dt_max is None:
				_dt_max = dates_rounded
			else:
				_dt_max = max(_dt_max, dates_rounded)

			if dates_rounded in _dt_cnt:
				_dt_cnt[dates_rounded] += 1
			else:
				_dt_cnt[dates_rounded] = 1
	print "_dt_min: %s" % _dt_min
	print "_dt_max: %s" % _dt_max


def Write():
	fn = Conf.fn_output
	with open(fn, "w") as fo:
		fo.write("# datetime cnt\n")

		dt = _dt_min
		while True:
			if dt in _dt_cnt:
				fo.write("%s %d\n" % (dt, _dt_cnt[dt]))
			else:
				fo.write("%s 0\n" % (dt))
			if dt == _dt_max:
				break
			dt += + datetime.timedelta(days=7)
	print "Created %s %d" % (fn, os.path.getsize(fn))


def main(argv):
	Conf.ParseOpts(argv)
	ReadTweetsBucketizeDate()
	Write()


if __name__ == "__main__":
  sys.exit(main(sys.argv))
