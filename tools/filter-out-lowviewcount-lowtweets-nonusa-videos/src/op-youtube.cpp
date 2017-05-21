#include <fstream>
#include <boost/format.hpp>
#include "op-youtube.h"
#include "util.h"

using namespace std;


OpYoutube::OpYoutube(ifstream& ifs) {
	ifs.read((char*)&id, sizeof(id));

	ifs.read((char*)&uid, sizeof(uid));

	Util::ReadStr(ifs, created_at);
	//created_at_pt = boost::posix_time::time_from_string(created_at);

	ifs.read((char*)&lat, sizeof(lat));
	ifs.read((char*)&lon, sizeof(lon));
	Util::ReadStr(ifs, obj_id);
	ifs.read((char*)&youtube_video_uploader, sizeof(youtube_video_uploader));

	size_t topic_cnt;
	ifs.read((char*)&topic_cnt, sizeof(topic_cnt));
	for (size_t i = 0; i < topic_cnt; i ++) {
		string t;
		Util::ReadStr(ifs, t);
		topics.push_back(t);
	}
	ifs.read((char*)&type, sizeof(type));

	ifs.read((char*)&in_usa, sizeof(in_usa));
}


void OpYoutube::Write(ofstream& ofs) {
	ofs.write((char*)&id, sizeof(id));

	ofs.write((char*)&uid, sizeof(uid));

	Util::WriteStr(ofs, created_at);

	ofs.write((char*)&lat, sizeof(lat));
	ofs.write((char*)&lon, sizeof(lon));
	Util::WriteStr(ofs, obj_id);
	ofs.write((char*)&youtube_video_uploader, sizeof(youtube_video_uploader));

	size_t topic_cnt = topics.size();
	ofs.write((char*)&topic_cnt, sizeof(topic_cnt));
	for (const string& t: topics)
		Util::WriteStr(ofs, t);

	ofs.write((char*)&type, sizeof(type));
	ofs.write((char*)&in_usa, sizeof(in_usa));
}


std::string OpYoutube::to_string() const {
	return str(
			boost::format("%d %f %f %s %d %d %s %d %c")
			% type
			% lat
			% lon
			% obj_id
			% id	// tweet id
			% uid
			% created_at
			% youtube_video_uploader
			% in_usa
			);
}


ostream& operator<< (ostream& os, const OpYoutube& op) {
	os << boost::format("%ld %10ld %s %s %10lf %11lf %s %c")
		% op.id
		% op.uid
		% op.created_at
		% op.type
		% op.lat
		% op.lon
		% op.obj_id
		% op.in_usa;
	return os;
}
