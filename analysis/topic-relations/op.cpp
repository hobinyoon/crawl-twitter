#include <fstream>
#include <set>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include "conf.h"
#include "dc.h"
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

	void StatNumTopicsInATweet() {
		// histogram of the number of topics (that is bigger than 1) in a tweet
		map<size_t, int> histo;

		for (auto& e: _entries) {
			vector<string>& topics = e->topics;
			size_t s = topics.size();

			auto it = histo.find(s);
			if (it == histo.end()) {
				histo[s] = 1;
			} else {
				it->second ++;
			}
		}

		cout << "# num_topics num percent\n";
		for (auto i: histo)
			cout << boost::format("%2d %6d %5.2f%%\n")
				% i.first % i.second % (100.0 * i.second / _entries.size());
	}

	void TopicRelations() {
		// count topics only once for the same videos
		list<Entry*> entries;
		{
			set<string> vids;
			for (auto& e: _entries) {
				string& vid = e->youtube_video_id;
				auto it = vids.find(vid);
				if (it == vids.end()) {
					entries.push_back(e);
					vids.insert(vid);
				}
			}
		}

		struct Key {
			string a;
			string b;

			Key(const string& a_, const string& b_) {
				a = std::min(a_, b_);
				b = std::max(a_, b_);
			}

			bool operator < (const Key& r) const {
				if (a < r.a) return true;
				if (a > r.a) return false;
				return (b < r.b);
			}
		};

		map<Key, int> key_cnt;
		map<string, int> topic_cnt;

		for (auto& e: entries) {
			vector<string>& topics = e->topics;

			for (auto& t: topics) {
				auto it = topic_cnt.find(t);
				if (it == topic_cnt.end()) {
					topic_cnt[t] = 1;
				} else {
					it->second ++;
				}
			}

			size_t s = topics.size();
			if (s <= 1)
				continue;

			// count occurrances of all combinations of pairs of topics
			for (size_t i = 0; i < s; i ++) {
				for (size_t j = i + 1; j < s; j ++) {
					//cout << i << " " << j << "\n";
					if (topics[i] == topics[j])
						continue;
					Key k(topics[i], topics[j]);
					auto it = key_cnt.find(k);
					if (it == key_cnt.end()) {
						key_cnt[k] = 1;
					} else {
						it->second ++;
					}
				}
			}
		}

		map<int, vector<Key> > cnt_key;
		for (auto i: key_cnt) {
			const Key& k = i.first;
			int cnt = i.second;

			auto it = cnt_key.find(cnt);
			if (it == cnt_key.end()) {
				vector<Key> v;
				v.push_back(k);
				cnt_key[cnt] = v;
			} else {
				cnt_key[cnt].push_back(k);
			}
		}

		cout << "# topic1             topic2             cnt_and  cnt1 cnt2  (1+2)/1 (1+2)/2\n";
		int cnt = 0;
		for (auto it = cnt_key.rbegin(); it != cnt_key.rend(); it ++ ) {
			for (auto k: it->second) {
				cout << boost::format("%-20s %-20s %5d %5d %5d %6.2f %6.2f\n")
					% k.a % k.b % it->first % topic_cnt[k.a] % topic_cnt[k.b]
					% (100.0 * it->first / topic_cnt[k.a])
					% (100.0 * it->first / topic_cnt[k.b]);
				if (Conf::max_output_lines != 0) {
					cnt ++;
					if (cnt == Conf::max_output_lines)
						break;
				}
			}

			if (Conf::max_output_lines != 0 && cnt == Conf::max_output_lines)
				break;
		}

		//for (auto i: cnt_key) {
		//	for (auto k: i.second) {
		//		cout << boost::format("%20s %20s %5d %5d %5d %6.2f %6.2f\n")
		//			% k.a % k.b % i.first % topic_cnt[k.a] % topic_cnt[k.b]
		//			% (100.0 * i.first / topic_cnt[k.a])
		//			% (100.0 * i.first / topic_cnt[k.b]);
		//		cnt ++;
		//		if (cnt == 10)
		//			break;
		//	}
		//}

		//int cnt = 0;
		//for (auto i: key_cnt) {
		//	cout << boost::format("%20s %20s %5d %5d %5d %6.2f %6.2f\n")
		//		% i.first.a % i.first.b % i.second % topic_cnt[i.first.a] % topic_cnt[i.first.b]
		//		% (100.0 * i.second / topic_cnt[i.first.a])
		//		% (100.0 * i.second / topic_cnt[i.first.b]);
		//	cnt ++;
		//	if (cnt == 10)
		//		break;
		//}
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
