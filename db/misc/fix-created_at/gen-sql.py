#! /usr/bin/python

import sys


_INCO_IDS = set()

def _ReadInco():
	with open("tweets-created_at-incorrect") as f:
		for line in f:
			t = line.split("\t")
			if len(t) != 2:
				raise RuntimeError("Unexpceted format: %s" % line)
			_INCO_IDS.add(long(t[0]))
	print "-- %d incorrect ids." % len(_INCO_IDS)


_CO_ID_CA = {}

def _ReadCo():
	with open("tweets-created_at-correct") as f:
		for line in f:
			line = line.rstrip()
			t = line.split("\t")
			if len(t) != 2:
				raise RuntimeError("Unexpceted format: %s" % line)
			id_ = long(t[0])
			ca = t[1]
			_CO_ID_CA[id_] = ca
			#print "%d %s" % (id_, ca)
	print "-- %d correct (id, ca)s." % len(_CO_ID_CA)


def _GenSQL():
	for icid in _INCO_IDS:
		# this is checked when _CO_ID_CA[icid]
		#if icid not in _CO_ID_CA:
		#	raise RuntimeError("icid %d is not in _CO_ID_CA" % icid)
		ca = _CO_ID_CA[icid]
		print "update tweets set created_at='%s' where id=%d;" % (ca, icid)


def main(argv):
	# Read the incorrect file
	#   store id
	# Read the correct file
	#   save id, created_at when the id is in the incorrect file
	# Write the output
	#   with the progress status
	_ReadInco()
	_ReadCo()
	_GenSQL()


if __name__ == "__main__":
	sys.exit(main(sys.argv))
