// sudo apt-get install -y libmysqlcppconn-dev
// http://dev.mysql.com/doc/connector-cpp/en/connector-cpp-examples-complete-example-1.html

#include <fstream>
#include <set>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include "conf.h"
#include "dc.h"
#include "op.h"
#include "stat.h"
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
		string fn;
		if (Conf::max_repeated_videos_per_user == 1) {
			fn = Conf::fn_tweets_1rv_per_user;
		} else {
			fn = Conf::fn_tweets;
		}
		Util::CpuTimer _(str(boost::format("Loading tweets from file %s ...\n") % fn), 2);

		ifstream ifs(fn.c_str(), ios::binary);
		if (! ifs.is_open())
			throw runtime_error(str(boost::format("unable to open file %1%") % fn));

		size_t e_size;
		ifs.read((char*)&e_size, sizeof(e_size));
		for (size_t i = 0; i < e_size; i ++)
			_entries.push_back(new Entry(ifs));
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

	void _FilterOutRepeatedAccessFromSameUser() {
		if (Conf::max_repeated_videos_per_user == -1)
			return;
		if (Conf::max_repeated_videos_per_user == 1) {
			// there is already input file for this
			return;
		}

		Util::CpuTimer _(str(boost::format("Allow at most %d repeated access "
						"to the same video from the same user ...\n") % Conf::max_repeated_videos_per_user), 2);

		struct Key {
			long uid;
			string vid;	// youtube video id

			Key(Entry* e)
				: uid(e->uid), vid(e->youtube_video_id)
			{}

			bool operator< (const Key& r) const {
				if (uid < r.uid) return true;
				if (uid > r.uid) return false;
				return vid < r.vid;
			}
		};

		size_t before = _entries.size();

		map<Key, int> uid_vids_cnt;
		for (auto it = _entries.begin(); it != _entries.end(); ) {
			Key k(*it);
			int cnt = -1;
			if (uid_vids_cnt.find(k) == uid_vids_cnt.end()) {
				uid_vids_cnt[k] = 1;
				cnt = 1;
			} else {
				cnt = uid_vids_cnt[k] + 1;
				uid_vids_cnt[k] = cnt;
			}

			if (cnt > Conf::max_repeated_videos_per_user) {
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
		_FilterOutRepeatedAccessFromSameUser();
	}

	struct DcNum {
		struct Val {
			int vids = 0;
			int uids = 0;
			int topics = 0;
		};

		// map<dc_name, Val>
		map<string, Val> _entries;

		void SetVids(const string& dc_name, int i) {
			auto it = _entries.find(dc_name);
			if (it == _entries.end()) {
				Val v;
				v.vids = i;
				_entries[dc_name] = v;
			} else {
				it->second.vids = i;
			}
		}

		void SetUids(const string& dc_name, int i) {
			auto it = _entries.find(dc_name);
			if (it == _entries.end()) {
				Val v;
				v.uids = i;
				_entries[dc_name] = v;
			} else {
				it->second.uids = i;
			}
		}

		void SetTopics(const string& dc_name, int i) {
			auto it = _entries.find(dc_name);
			if (it == _entries.end()) {
				Val v;
				v.topics = i;
				_entries[dc_name] = v;
			} else {
				it->second.topics = i;
			}
		}
	};

	ostream& operator<< (ostream& os, DcNum& d) {
		os << boost::format("# %-8s %6s %6s %6s\n")
			% "dc_name" % "vids" % "uids" % "topics";

		for (auto e: d._entries) {
			if (e.first == "All")
				continue;
			if (e.first == "Avg")
				continue;
			os << boost::format("%-10s %6d %6d %6d\n")
				% e.first
				% e.second.vids
				% e.second.uids
				% e.second.topics;
		}
		//auto e = d._entries["All"];
		//os << boost::format("%-10s %6d %6d %6d\n")
		//	% "All"
		//	% e.vids
		//	% e.uids
		//	% e.topics;
		auto e = d._entries["Avg"];
		os << boost::format("%-10s %6d %6d %6d\n")
			% "Avg"
			% e.vids
			% e.uids
			% e.topics;

		return os;
	}

	DcNum _dc_num;

	void _CntNumUniqVideosPerDC() {
		Util::CpuTimer _("Num uniq videos per DC ...\n");

		map<DC*, set<string> > dc_vids;
		for (auto e: _entries) {
			DC* dc = DCs::GetClosest(e->geo_lati, e->geo_longi);
			const string& vid = e->youtube_video_id;

			auto it = dc_vids.find(dc);
			if (it == dc_vids.end()) {
				set<string> s;
				s.insert(vid);
				dc_vids[dc] = s;
			} else {
				it->second.insert(vid);
			}
		}

		int cnt_all = 0;
		for (auto i: dc_vids) {
			cnt_all += i.second.size();
			_dc_num.SetVids(i.first->name, i.second.size());
		}
		_dc_num.SetVids("All", cnt_all);
		_dc_num.SetVids("Avg", cnt_all / DCs::GetAll().size());
	}

	void _CntNumUniqUsersPerDC() {
		Util::CpuTimer _("Num uniq users per DC ...\n");

		map<DC*, set<long> > dc_uids;
		for (auto e: _entries) {
			DC* dc = DCs::GetClosest(e->geo_lati, e->geo_longi);

			auto it = dc_uids.find(dc);
			if (it == dc_uids.end()) {
				set<long> s;
				s.insert(e->uid);
				dc_uids[dc] = s;
			} else {
				it->second.insert(e->uid);
			}
		}

		int cnt_all = 0;
		for (auto i: dc_uids) {
			cnt_all += i.second.size();
			_dc_num.SetUids(i.first->name, i.second.size());
		}
		_dc_num.SetUids("All", cnt_all);
		_dc_num.SetUids("Avg", cnt_all / DCs::GetAll().size());
	}

	void _CntNumUniqTopicsPerDC() {
		Util::CpuTimer _("Num uniq topics per DC ...\n");

		map<DC*, set<string> > dc_topics;
		for (auto e: _entries) {
			DC* dc = DCs::GetClosest(e->geo_lati, e->geo_longi);

			auto it = dc_topics.find(dc);
			if (it == dc_topics.end()) {
				set<string> s;
				for (auto& t: e->topics)
					s.insert(t);
				dc_topics[dc] = s;
			} else {
				for (auto& t: e->topics)
					it->second.insert(t);
			}
		}

		int cnt_all = 0;
		for (auto i: dc_topics) {
			cnt_all += i.second.size();
			_dc_num.SetTopics(i.first->name, i.second.size());
		}
		_dc_num.SetTopics("All", cnt_all);
		_dc_num.SetTopics("Avg", cnt_all / DCs::GetAll().size());
	}


	void CntNumUniq() {
		_CntNumUniqVideosPerDC();
		_CntNumUniqUsersPerDC();
		_CntNumUniqTopicsPerDC();

		cout << _dc_num;

		const string& fn = Conf::fn_num_uniq_attrs_per_dc;
		ofstream ofs(fn);
		if (! ofs.is_open())
			throw runtime_error(str(boost::format("unable to open file %1%") % fn));
		ofs << _dc_num;
	}

	void FreeMem() {
		for (auto e: _entries)
			delete e;
	}

	boost::posix_time::ptime oldest_created_at;
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
	static const char* s[] = {"W", "R"};
	os << s[opt];
	return os;
}
