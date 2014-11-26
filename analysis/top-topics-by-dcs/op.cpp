#include <fstream>
#include <set>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include "conf.h"
#include "dc.h"
#include "op.h"
#include "topic-filter.h"
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

	// map<DC*, map<topic, cnt> >
	map<DC*, map<string, int> > _dc_topic_cnt;

	void _PrintTopTopics() {
		struct TopicCnt {
			string topic;
			int cnt;

			TopicCnt(const string& t, int c)
			: topic(t), cnt(c)
			{}

			bool operator < (const TopicCnt& r) const {
				if (cnt < r.cnt) return true;
				if (cnt > r.cnt) return false;
				return (topic < r.topic);
			}
		};

		stringstream ss;

		for (auto dtc: _dc_topic_cnt) {
			ss << dtc.first->name << "\n";

			set<TopicCnt> tc_set;
			for (auto tc: dtc.second)
				tc_set.insert(TopicCnt(tc.first, tc.second));

			int c = 0;
			for (auto i = tc_set.rbegin(); i != tc_set.rend(); ++ i) {
				ss << "  " << i->topic << " " << i->cnt << "\n";
				c ++;
				if (c == Conf::max_topics_per_dc)
					break;
			}
		}

		const string& fn = Conf::fn_top_topics_by_dcs;
		ofstream ofs(fn);
		if (! ofs.is_open())
			throw runtime_error(str(boost::format("Unable to open file %s") % fn));
		ofs << ss.str();
		cout << "  Created file " << fn << "\n";
	}

	void CntTopicsByDCs() {
		Util::CpuTimer _("Counting topics by DCs ...\n");

		for (auto& e: _entries) {
			DC* dc = DCs::GetClosest(e->geo_lati, e->geo_longi);
			
			auto it = _dc_topic_cnt.find(dc);
			if (it == _dc_topic_cnt.end()) {
				map<string, int> m;
				for (auto& t: e->topics) {
					if (TopicFilter::IsBlackListed(t))
						continue;
					m[t] = 1;
				}
				_dc_topic_cnt[dc] = m;
			} else {
				map<string, int>& m = it->second;
				for (auto& t: e->topics) {
					if (TopicFilter::IsBlackListed(t))
						continue;
					auto it2 = m.find(t);
					if (it2 == m.end()) {
						m[t] = 1;
					} else {
						it2->second ++;
					}
				}
			}
		}

		_PrintTopTopics();
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
	static const char* s[] = {"W", "R"};
	os << s[opt];
	return os;
}
