#include <fstream>
#include <set>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include "conf.h"
#include "tweet.h"
#include "util.h"

using namespace std;


Tweet::Tweet(ifstream& ifs) {
	ifs.read((char*)&id, sizeof(id));
	ifs.read((char*)&uid, sizeof(uid));

	Util::ReadStr(ifs, created_at_str);
	// Takes like 11sec for the 100% dataset
	//created_at = boost::posix_time::time_from_string(created_at_str);

	ifs.read((char*)&geo_lati, sizeof(geo_lati));
	ifs.read((char*)&geo_longi, sizeof(geo_longi));
	Util::ReadStr(ifs, youtube_video_id);
	ifs.read((char*)&youtube_video_uploader, sizeof(youtube_video_uploader));

	size_t topic_cnt;
	ifs.read((char*)&topic_cnt, sizeof(topic_cnt));
	for (size_t i = 0; i < topic_cnt; i ++) {
		string t;
		Util::ReadStr(ifs, t);
		topics.push_back(t);
	}
	ifs.read((char*)&type, sizeof(type));
}


void Tweet::Write(ofstream& ofs) {
	ofs.write((char*)&id, sizeof(id));
	ofs.write((char*)&uid, sizeof(uid));

	Util::WriteStr(ofs, created_at_str);

	ofs.write((char*)&geo_lati, sizeof(geo_lati));
	ofs.write((char*)&geo_longi, sizeof(geo_longi));
	Util::WriteStr(ofs, youtube_video_id);;
	ofs.write((char*)&youtube_video_uploader, sizeof(youtube_video_uploader));

	size_t topic_cnt = topics.size();
	ofs.write((char*)&topic_cnt, sizeof(topic_cnt));
	for (const auto& tp: topics)
		Util::WriteStr(ofs, tp);
	ofs.write((char*)&type, sizeof(type));
}


ostream& operator<< (ostream& os, const Tweet& t) {
	os << t.id;
	os << boost::format(" %10d") % t.uid;
	os << " " << t.created_at_str;
	os << boost::format(" %8.3f") % t.geo_lati;
	os << boost::format(" %8.3f") % t.geo_longi;
	os << " " << t.youtube_video_id;
	os << boost::format(" %10d") % t.youtube_video_uploader;
	for (const auto& tp: t.topics)
		os << " " << tp;
	os << " " << t.type;
	return os;
}
