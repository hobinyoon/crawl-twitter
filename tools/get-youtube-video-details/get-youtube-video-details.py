#!/usr/bin/env python

import os
import pprint
import sys

import cPickle as pickle

sys.path.insert(0, "%s/work/mutant/ec2-tools/lib/util" % os.path.expanduser("~"))
import Cons

import YoutubeListVideos

def main(argv):
	ids = ["Ks-_Mh1QhMc", "c0KYU2j0TM4", "eIho2S0ZahI"]
	vlist = YoutubeListVideos.List(ids)
	#pprint.pprint(vlist)

	fn = "video-details.pkl"
	with open(fn, "wb") as fo:
		for i in vlist["items"]:
			pickle.dump(i, fo, pickle.HIGHEST_PROTOCOL)
	Cons.P("Created %s %d" % (fn, os.path.getsize(fn)))


if __name__ == "__main__":
	sys.exit(main(sys.argv))
