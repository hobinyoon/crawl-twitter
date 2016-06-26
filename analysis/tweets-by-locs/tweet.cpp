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

//	Tweet::Tweet(long id_,
//			long uid_,
//			const string& created_at_,
//			double geo_lati_,
//			double geo_longi_,
//			const string& youtube_video_id_,
//			const string& topics_)
//		: id(id_), uid(uid_), created_at_str(created_at_),
//		geo_lati(geo_lati_), geo_longi(geo_longi_),
//		youtube_video_id(youtube_video_id_)
//	{
//		created_at = boost::posix_time::time_from_string(created_at_str);
//
//		static const auto sep = boost::is_any_of(" ");
//		boost::split(topics, topics_, sep);
//	}

Tweet::Tweet(ifstream& ifs) {
	ifs.read((char*)&id, sizeof(id));
	ifs.read((char*)&uid, sizeof(uid));

	Util::ReadStr(ifs, created_at_str);
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
		// topics.push_back(t);
	}
	ifs.read((char*)&type, sizeof(type));
}
