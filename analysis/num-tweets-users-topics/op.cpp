#include <fstream>
#include <set>
#include <string>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include "conf.h"
#include "op.h"
#include "util.h"

using namespace std;

namespace Ops {
	void _ReadStr(ifstream& ifs, string& str) {
		size_t s;
		ifs.read((char*)&s, sizeof(s));
		str.resize(s);
		ifs.read((char*)&str[0], s);
	}

	Entry::Entry(long id_,
			long uid_,
			const string& created_at_,
			double geo_lati_,
			double geo_longi_,
			const string& youtube_video_id_,
			const string& topics_)
		: id(id_), uid(uid_), created_at_str(created_at_),
		geo_lati(geo_lati_), geo_longi(geo_longi_),
		youtube_video_id(youtube_video_id_)
	{
		created_at = boost::posix_time::time_from_string(created_at_str);

		static const auto sep = boost::is_any_of(" ");
		boost::split(topics, topics_, sep);
	}

	Entry::Entry(ifstream& ifs) {
		ifs.read((char*)&id, sizeof(id));
		ifs.read((char*)&uid, sizeof(uid));

		_ReadStr(ifs, created_at_str);
		created_at = boost::posix_time::time_from_string(created_at_str);

		ifs.read((char*)&geo_lati, sizeof(geo_lati));
		ifs.read((char*)&geo_longi, sizeof(geo_longi));
		_ReadStr(ifs, youtube_video_id);
		ifs.read((char*)&youtube_video_uploader, sizeof(youtube_video_uploader));

		size_t topic_cnt;
		ifs.read((char*)&topic_cnt, sizeof(topic_cnt));
		for (size_t i = 0; i < topic_cnt; i ++) {
			string t;
			_ReadStr(ifs, t);
			topics.push_back(t);
		}
		ifs.read((char*)&type, sizeof(type));
	}

	static list<Entry*> _entries;

	void _LoadTweetsFromFile() {
		const string& fn = Conf::fn_tweets;
		Util::CpuTimer _(str(boost::format("Loading tweets from file %s ...\n") % fn), 2);

		ifstream ifs(fn.c_str(), ios::binary);
		if (! ifs.is_open())
			throw runtime_error(str(boost::format("unable to open file %1%") % fn));

		size_t e_size;
		ifs.read((char*)&e_size, sizeof(e_size));
		for (size_t i = 0; i < e_size; i ++) {
			_entries.push_back(new Entry(ifs));

			if (Conf::partial_load > 0 && i == Conf::partial_load)
				break;
		}
		cout << "    _entries.size()=" << _entries.size() << "\n";
	}

	void _FilterOutWrites() {
		Util::CpuTimer _("Filter out writes ...\n", 2);

		size_t before = _entries.size();

		map<string, int> vids_cnt;
		for (auto it = _entries.begin(); it != _entries.end(); ) {
			int cnt = -1;
			if (vids_cnt.find((*it)->youtube_video_id) == vids_cnt.end()) {
				cnt = 1;
			} else {
				cnt = vids_cnt[(*it)->youtube_video_id] + 1;
			}
			vids_cnt[(*it)->youtube_video_id] = cnt;

			if (cnt == 1) {
				it = _entries.erase(it);
			} else {
				it ++;
			}
		}

		cout << boost::format("    _entries.size()= before %d, after %d (%.2f%%)\n")
			% before % _entries.size()
			% (100.0 * _entries.size() / before);
	}

	void Load() {
		Util::CpuTimer _("Loading tweets ...\n");

		_LoadTweetsFromFile();
		//_FilterOutWrites();
	}

	void Cnt() {
		Util::CpuTimer _("Counting ...\n");

		set<string> vids;
		set<string> topics;
		set<long> uids;

		for (auto& e: _entries) {
			uids.insert(e->uid);
			vids.insert(e->youtube_video_id);

			for (auto& t: e->topics)
				topics.insert(t);
		}

		cout << boost::format(
				"  num_videos=%7d\n"
				"  num_ops=   %7d %7.2f\n"
				"  num_topics=%7d %7.2f\n"
				"  num_users= %7d %7.2f\n")
			%	vids.size()
			%	_entries.size() %	(double(_entries.size()) / vids.size())
			%	topics.size() %	(double(topics.size()) / vids.size())
			%	uids.size() %	(double(uids.size()) / vids.size());
	}

	void FreeMem() {
		for (auto e: _entries)
			delete e;
	}
}


std::ostream& operator<< (std::ostream& os, const Ops::Entry& op) {
	os << boost::format("%ld %10ld %s %s %10lf %11lf %s")
		% op.id
		% op.uid
		% op.created_at
		% op.type
		% op.geo_lati
		% op.geo_longi
		% op.youtube_video_id;
	return os;
}


std::ostream& operator<< (std::ostream& os, const Ops::Type& opt) {
	static const char* s[] = {"NA", "W", "R"};
	os << s[opt];
	return os;
}
