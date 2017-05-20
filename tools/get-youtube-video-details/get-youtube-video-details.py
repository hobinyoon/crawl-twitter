#!/usr/bin/env python

import os
import pprint
import sys

import cPickle as pickle

sys.path.insert(0, "%s/work/mutant/ec2-tools/lib/util" % os.path.expanduser("~"))
import Cons

import YoutubeListVideos

def GetVideoDetails():
	ids = ["Ks-_Mh1QhMc", "c0KYU2j0TM4", "eIho2S0ZahI"]
	vlist = YoutubeListVideos.List(ids)
	#pprint.pprint(vlist)

	fn = "video-details.pkl"
	with open(fn, "wb") as fo:
		items = vlist["items"]
		fo.write("%d\n" % len(items))
		for i in items:
			pickle.dump(i, fo, pickle.HIGHEST_PROTOCOL)
	Cons.P("Created %s %d" % (fn, os.path.getsize(fn)))


def ReadVideoDetails():
	fn = "video-details.pkl"
	with open(fn, "rb") as fo:
		num_videos = int(fo.readline())
		for i in range(num_videos):
			v = pickle.load(fo)
			pprint.pprint(v)


def main(argv):
	GetVideoDetails()
	#ReadVideoDetails()


if __name__ == "__main__":
	sys.exit(main(sys.argv))
