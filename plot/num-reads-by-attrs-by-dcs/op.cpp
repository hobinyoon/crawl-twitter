// sudo apt-get install -y libmysqlcppconn-dev
// http://dev.mysql.com/doc/connector-cpp/en/connector-cpp-examples-complete-example-1.html

#include <fstream>
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
		_FilterOutWrites();
		_FilterOutRepeatedAccessFromSameUser();
	}

	void _NumReadsByVideos() {
		Util::CpuTimer _("Num reads by videos ...\n");

		// map<youtube_video_id, vector<Entry*> >
		map<string, vector<Entry*> > by_youtube_video_ids;
		for (auto e: _entries) {
			auto it = by_youtube_video_ids.find(e->youtube_video_id);
			if (it == by_youtube_video_ids.end()) {
				vector<Entry*> v;
				v.push_back(e);
				by_youtube_video_ids[e->youtube_video_id] = v;
			} else {
				it->second.push_back(e);
			}
		}

		vector<size_t> sizes;
		for (auto i: by_youtube_video_ids) {
			if (i.second.size() > 1)
				sizes.push_back(i.second.size() - 1);
		}

		double x[10];
		double y[10];
		Stat::CDF(sizes, x, y, true, 2);
	}

	void _NumReadsByUsers() {
		Util::CpuTimer _("Num reads by users ...\n");

		map<long, vector<Entry*> > by_uids;
		for (auto e: _entries) {
			auto it = by_uids.find(e->uid);
			if (it == by_uids.end()) {
				vector<Entry*> v;
				v.push_back(e);
				by_uids[e->uid] = v;
			} else {
				it->second.push_back(e);
			}
		}

		vector<size_t> sizes;
		for (auto i: by_uids) {
			if (i.second.size() > 1)
				sizes.push_back(i.second.size() - 1);
		}

		double x[10];
		double y[10];
		Stat::CDF(sizes, x, y, true, 2);
	}

	void _NumReadsByTopics() {
		Util::CpuTimer _("Num reads by topics ...\n");

		map<string, vector<Entry*> > by_topics;
		for (auto e: _entries) {
			for (auto& t: e->topics) {
				auto it = by_topics.find(t);
				if (it == by_topics.end()) {
					vector<Entry*> v;
					v.push_back(e);
					by_topics[t] = v;
				} else {
					it->second.push_back(e);
				}
			}
		}

		vector<size_t> sizes;
		for (auto i: by_topics) {
			if (i.second.size() > 1)
				sizes.push_back(i.second.size() - 1);
		}

		double x[10];
		double y[10];
		Stat::CDF(sizes, x, y, true, 2);
	}

	void _NumReadsByVideosByDCs() {
		Util::CpuTimer _("Num reads by videos by DCs ...\n");

		struct Key {
			string vid;
			DC* dc;

			Key(const string vid_, DC* dc_)
				: vid(vid_), dc(dc_)
			{}

			bool operator< (const Key& r) const {
				int c = vid.compare(r.vid);
				if (c < 0) return true;
				else if (c > 0) return false;

				return dc < r.dc;
			}
		};

		map<Key, int> by_keys;
		for (auto e: _entries) {
			Key k(e->youtube_video_id, DCs::GetClosest(e->geo_lati, e->geo_longi));
			auto it = by_keys.find(k);
			if (it == by_keys.end()) {
				by_keys[k] = 1;
			} else {
				it->second ++;
			}
		}

		vector<size_t> sizes;
		for (auto i: by_keys)
			sizes.push_back(i.second);

		double x[10];
		double y[10];
		Stat::CDF(sizes, x, y, true, 2);
	}

	void _NumReadsByUsersByDCs() {
		Util::CpuTimer _("Num reads by users by DCs ...\n");

		struct Key {
			long uid;
			DC* dc;

			Key(const long uid_, DC* dc_)
				: uid(uid_), dc(dc_)
			{}

			bool operator< (const Key& r) const {
				if (uid < r.uid) return true;
				else if (uid > r.uid) return false;

				return dc < r.dc;
			}
		};

		map<Key, int> by_keys;
		for (auto e: _entries) {
			Key k(e->youtube_video_uploader, DCs::GetClosest(e->geo_lati, e->geo_longi));
			auto it = by_keys.find(k);
			if (it == by_keys.end()) {
				by_keys[k] = 1;
			} else {
				it->second ++;
			}
		}

		vector<size_t> sizes;
		for (auto i: by_keys)
			sizes.push_back(i.second);

		double x[10];
		double y[10];
		Stat::CDF(sizes, x, y, true, 2);
	}

	void _NumReadsByTopicsByDCs() {
		Util::CpuTimer _("Num reads by topics by DCs ...\n");

		struct Key {
			string topic;
			DC* dc;

			Key(const string topic_, DC* dc_)
				: topic(topic_), dc(dc_)
			{}

			bool operator< (const Key& r) const {
				int c = topic.compare(r.topic);
				if (c < 0) return true;
				else if (c > 0) return false;

				return dc < r.dc;
			}
		};

		map<Key, int> by_keys;
		for (auto e: _entries) {
			for (auto& t: e->topics) {
				Key k(t, DCs::GetClosest(e->geo_lati, e->geo_longi));
				auto it = by_keys.find(k);
				if (it == by_keys.end()) {
					by_keys[k] = 1;
				} else {
					it->second ++;
				}
			}
		}

		vector<size_t> sizes;
		for (auto i: by_keys)
			sizes.push_back(i.second);

		double x[10];
		double y[10];
		Stat::CDF(sizes, x, y, true, 2);
	}

	void NumReadsBy() {
		_NumReadsByVideosByDCs();
		_NumReadsByUsersByDCs();
		_NumReadsByTopicsByDCs();
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
