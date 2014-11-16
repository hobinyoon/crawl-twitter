#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include "tweet.h"
#include "util.h"

using namespace std;


Tweet::Tweet(long id_,
		long uid_,
		const string& created_at_,
		double geo_lati_,
		double geo_longi_,
		const string& youtube_video_id_,
		const string& topics_)
: id(id_), uid(uid_), created_at(created_at_),
	geo_lati(geo_lati_), geo_longi(geo_longi_),
	youtube_video_id(youtube_video_id_),
	type(Type::NA)
{
	static const auto sep = boost::is_any_of(" ");

	string t = topics_;
	boost::algorithm::to_lower(t);
	boost::split(topics, t, sep);
}


void Tweet::Write(ofstream& ofs) {
	ofs.write((char*)&id, sizeof(id));
	ofs.write((char*)&uid, sizeof(uid));
	Util::WriteStr(ofs, created_at);
	ofs.write((char*)&geo_lati, sizeof(geo_lati));
	ofs.write((char*)&geo_longi, sizeof(geo_longi));
	Util::WriteStr(ofs, youtube_video_id);
	ofs.write((char*)&youtube_video_uploader, sizeof(youtube_video_uploader));
	{
		size_t s = topics.size();
		ofs.write((char*)&s, sizeof(s));
	}
	for (auto& t: topics)
		Util::WriteStr(ofs, t);
	ofs.write((char*)&type, sizeof(type));
}


std::ostream& operator<< (std::ostream& os, const Tweet& t) {
	os << boost::format("%ld %10ld %s %s %10lf %11lf %s")
		% t.id
		% t.uid
		% t.created_at
		% t.type
		% t.geo_lati
		% t.geo_longi
		% t.youtube_video_id;

	os << " [";
	int cnt = 0;
	for (auto topic: t.topics) {
		if (cnt > 0)
			os << " ";
		os << topic;
		cnt ++;
	}
	os << "]";

	return os;
}


std::ostream& operator<< (std::ostream& os, const Tweet::Type& t) {
	static const char* s[] = {"NA", "W", "R"};
	os << s[t];
	return os;
}
