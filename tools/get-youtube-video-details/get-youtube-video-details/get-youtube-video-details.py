#!/usr/bin/env python

import codecs
import os
import pprint
import re
import sys

import cPickle as pickle

sys.path.insert(0, "%s/work/mutant/ec2-tools/lib/util" % os.path.expanduser("~"))
import Cons

import YoutubeListVideos


def _QueryVideoDetails():
	with Cons.MT("Querying video details ..."):
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

			# got can be less than queried: there are videos unavailable or private
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


def _ReadVideoDetails():
	fn = "video-details.pkl"
	# {video_id, VideoInfo}
	vinfo = {}
	with open(fn, "rb") as fo:
		num_videos = int(fo.readline())
		for i in range(num_videos):
			v = pickle.load(fo)
			#pprint.pprint(v)
			v1 = v["contentDetails"]
			if v1["dimension"] != "2d":
				Cons.P("dimension=%s" % v1["dimension"])
			def_ = v1["definition"]
			dur = v1["duration"]
			id_ = v["id"]
			v1 = v["snippet"]
			title = v1["title"]
			#title = ""

			view_cnt = None
			try:
				view_cnt = v["statistics"]["viewCount"]
			except KeyError as e:
				pass

			# title is in utf-8 and can't be converted to string. Interesting.
			#Cons.Pnnl("%s %10s %s %10s " % (id_, view_cnt, def_, dur))
			#print title
			vi = VideoInfo()
			vi.id_ = id_
			vi.view_cnt = view_cnt
			vi.def_ = def_
			vi.dur = dur
			vi.title = title
			vinfo[id_] = vi
	return vinfo


def GetVideoDetails():
	fn = "../get-vids-to-query/.result/video-list"
	if not os.path.exists(fn):
		_QueryVideoDetails()

	# {vid: VideoInfo}
	vinfo = {}
	# Sorted by num_locs desc
	vids = []
	with open(fn) as fo:
		for line in fo:
			if len(line) == 0:
				continue
			if line[0] == "#":
				continue
			t = re.split(r" +", line.strip())
			if len(t) != 2:
				raise RuntimeError("Unexpected")
			vid = t[0]
			num_locs = t[1]
			vi = VideoInfo()
			vi.id_ = vid
			vi.num_locs = num_locs
			vinfo[vid] = vi
			vids.append(vid)

	# {vid: VideoInfo}
	vinfo1 = _ReadVideoDetails()
	for vid, vi in vinfo1.iteritems():
		vinfo[vid].Update(vi)

	fn = "video-detailed-merged"
	with codecs.open(fn, "w", "utf-8") as fo:
		for vid in vids:
			vi = vinfo[vid]

			# Skip unavailable or private videos
			if vi.view_cnt is None:
				continue

			fo.write("%s %4s %10s %4s %10s " % (vi.id_, vi.num_locs, vi.view_cnt, vi.def_, vi.dur))
			#Cons.Pnnl("%s %4s %10s %4s %10s " % (vi.id_, vi.num_locs, vi.view_cnt, vi.def_, vi.dur))
			#print vi.title
			if vi.title is None:
				fo.write("None")
			else:
				fo.write(vi.title)
			fo.write("\n")


class VideoInfo:
	def __init__(self):
		self.id_      = None
		self.view_cnt = None
		self.def_     = None
		self.dur      = None
		self.title    = None
		self.num_locs = None

	def Update(self, vi):
		self.id_      = vi.id_
		self.view_cnt = vi.view_cnt
		self.def_     = vi.def_
		self.dur      = vi.dur
		self.title    = vi.title


def main(argv):
	GetVideoDetails()


if __name__ == "__main__":
	sys.exit(main(sys.argv))
