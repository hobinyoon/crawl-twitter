#!/usr/bin/env python

import codecs
import os
import pprint
import re
import shutil
import sys

import cPickle as pickle

sys.path.insert(0, "%s/work/mutant/ec2-tools/lib/util" % os.path.expanduser("~"))
import Cons

import YoutubeListVideos


def _QueryVideoDetails(vids_to_query):
	with Cons.MT("Querying video details ..."):
		i = 0
		got = 0
		vid_rvi = {}

		# {vid: RawVideoInfo}
		# We keep None(s) (unavailable or private videos) so that you don't query those repeatedly
		for vid in vids_to_query:
			vid_rvi[vid] = None

		while i < len(vids_to_query):
			j = min(len(vids_to_query), i + 50)
			vids0 = vids_to_query[i:j]

			#Cons.P(vids0)
			vlist = YoutubeListVideos.List(vids0)
			#pprint.pprint(vlist)
			got += len(vlist["items"])

			for rvi in vlist["items"]:
				id_ = rvi["id"]
				vid_rvi[id_] = rvi

			# got can be less than queried: there are videos unavailable or private
			Cons.Pnnl("to_query=%d queried=%d(%.2f%%) got=%d(success_ratio=%.2f%%)" \
					% (len(vids_to_query), j, (100.0 * j / len(vids_to_query)), got, (100.0 * got / j)))
			Cons.ClearLine()
			i += 50
		Cons.P("to_query=%d queried=%d(%.2f%%) got=%d(success_ratio=%.2f%%)" \
				% (len(vids_to_query), j, (100.0 * j / len(vids_to_query)), got, (100.0 * got / j)))

		return vid_rvi


_fn_video_details_pickled = "video-details.pkl"
_fn_video_info_concise = "video-info-concise"

def _ReadVideoDetailsFromPickledFile():
	# {video_id, RawVideoInfo}
	vinfo = {}
	if not os.path.exists(_fn_video_details_pickled):
		return vinfo
	with open(_fn_video_details_pickled, "rb") as fo:
		num_videos = int(fo.readline())
		for i in range(num_videos):
			vid = fo.readline().strip()
			v = pickle.load(fo)
			# Skip the newline character
			fo.seek(1, os.SEEK_CUR)
			vinfo[vid] = v
	return vinfo


def GenVideoDetailsFiles():
	# {vid: VideoInfo}
	vinfo = {}
	# Sorted by num_locs desc
	vids = []
	fn = "../get-vids-to-query/.result/video-list"
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

	# {vid: RawVideoInfo}
	vid_rvi = _ReadVideoDetailsFromPickledFile()
	for vid, rvi in vid_rvi.iteritems():
		vinfo[vid].Update(rvi)

	# Build to-query list
	vids_to_query = []
	for vid in vids:
		if vid not in vid_rvi:
			vids_to_query.append(vid)

	# For dev
	#vids_to_query = vids_to_query[0:50]

	if len(vids_to_query) > 0:
		vid_rvi_new = _QueryVideoDetails(vids_to_query)
		for k, v in vid_rvi_new.iteritems():
			vid_rvi[k] = v

		with Cons.MT("Rewriting a new picked file"):
			fn_out_pickled_new = "video-details-new.pkl"
			with open(fn_out_pickled_new, "wb") as fo:
				fo.write("%d\n" % len(vid_rvi))
				for vid, rvi in vid_rvi.iteritems():
					fo.write("%s\n" % vid)
					pickle.dump(rvi, fo, pickle.HIGHEST_PROTOCOL)
					fo.write("\n")
			Cons.P("Created %s %d %d video infos" % (fn_out_pickled_new, os.path.getsize(fn_out_pickled_new), len(vid_rvi)))
			Cons.P("Replacing old file ...")
			shutil.move(fn_out_pickled_new, _fn_video_details_pickled)

		with Cons.MT("Rewriting a concise video info file ..."):
			for vid, rvi in vid_rvi_new.iteritems():
				vinfo[vid].Update(rvi)

			# Now, I kind of want to keep this in the database. Let's see how much time I have

			with codecs.open(_fn_video_info_concise, "w", "utf-8") as fo:
				for vid in vids:
					num_locs = None
					view_cnt = None
					def_     = None
					dur      = None
					title    = None
					vi = vinfo[vid]
					if vi is not None:
						num_locs = vi.num_locs
						view_cnt = vi.view_cnt
						def_     = vi.def_
						dur      = vi.dur
						title    = vi.title
					fo.write("%s %4s %10s %4s %10s %s\n" % (vid, num_locs, view_cnt, def_, dur, title))
			Cons.P("Created %s %d" % (_fn_video_info_concise, os.path.getsize(_fn_video_info_concise)))
	else:
		Cons.P("Neither %s nor %s was updated" % (_fn_video_details_pickled, _fn_video_info_concise))


class VideoInfo:
	def __init__(self):
		self.id_      = None
		self.view_cnt = None
		self.def_     = None
		self.dur      = None
		self.title    = None
		self.num_locs = None

	def Update(self, rvi):
		if rvi is None:
			return

		v1 = rvi["contentDetails"]
		if v1["dimension"] != "2d":
			Cons.P("Interesting: dimension=%s" % v1["dimension"])
		def_ = v1["definition"]
		dur = v1["duration"]
		id_ = rvi["id"]
		v1 = rvi["snippet"]
		# title is in utf-8
		title = v1["title"]

		view_cnt = None
		try:
			view_cnt = rvi["statistics"]["viewCount"]
		except KeyError as e:
			pass

		self.id_      = id_
		self.view_cnt = view_cnt
		self.def_     = def_
		self.dur      = dur
		self.title    = title


def main(argv):
	GenVideoDetailsFiles()


if __name__ == "__main__":
	sys.exit(main(sys.argv))
