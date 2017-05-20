#!/usr/bin/env python

import os
import pprint
import re
import sys

import cPickle as pickle

sys.path.insert(0, "%s/work/mutant/ec2-tools/lib/util" % os.path.expanduser("~"))
import Cons

import YoutubeListVideos

def GetVideoDetails():
	fn = "../get-vids-to-query/.result/video-list"
	vids = []
	with open(fn) as fo:
		for line in fo:
			if len(line) == 0:
				continue
			if line[0] == "#":
				continue
			t = re.split(r" +", line)
			if len(t) != 2:
				raise RuntimeError("Unexpected")
			vid = t[0]
			#num_locs = t[1]
			vids.append(vid)
	#Cons.P(vids)

	i = 0
	items = []
	got = 0
	while i < len(vids):
		j = min(len(vids), i + 50)
		vids0 = vids[i:j]

		#Cons.P(vids0)
		vlist = YoutubeListVideos.List(vids0)
		#pprint.pprint(vlist)
		got += len(vlist["items"])
		items += vlist["items"]
		Cons.Pnnl("to_query=%d queried=%d(%.2f%%) got=%d(success_ratio=%.2f%%)" \
				% (len(vids), j, (100.0 * j / len(vids)), got, (100.0 * got / j)))
		Cons.ClearLine()
		i += 50
	Cons.P("to_query=%d queried=%d(%.2f%%) got=%d(success_ratio=%.2f%%)" \
			% (len(vids), j, (100.0 * j / len(vids)), got, (100.0 * got / j)))

	fn = "video-details.pkl"
	with open(fn, "wb") as fo:
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
