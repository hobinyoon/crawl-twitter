#include <fstream>
#include <set>
#include <boost/format.hpp>

#include "cnt-attrs.h"
#include "conf.h"
#include "data-file-reader.h"
#include "dc.h"
#include "util.h"

using namespace std;

namespace CntAttrs {
	// uids ordered by crawled_at
	vector<long> _uids;

	map<long, vector<Tweet*> > _users_tweets;

	void Load() {
		Util::CpuTimer _("Loading ...\n");
		DataFileReader::GetUsersByCrawledAt(_uids);
		DataFileReader::GetTweets(_users_tweets);
	}

	void GetNumUniqAttrGrowth() {
		Util::CpuTimer _("Gen uniq attrs growth ...\n");

		// tweet count
		int t_cnt = 0;

		set<long> uids;
		set<string> vids;
		set<string> topics;

		const string& fn = Conf::fn_num_uniq_attrs_growth;
		ofstream ofs(fn);
		if (! ofs.is_open())
			throw runtime_error(str(boost::format("unable to open file %1%") % fn));
		ofs << "# vids tweets uids topics\n";

		size_t vids_size_prev = 0;

		for (auto uid: _uids) {
			auto it = _users_tweets.find(uid);
			if (it == _users_tweets.end())
				continue;

			uids.insert(uid);

			for (auto t: it->second) {
				vids.insert(t->youtube_video_id);
				topics.insert(t->topics.begin(), t->topics.end());
				t_cnt ++;

				if ((vids.size() % 10 == 0) && vids.size() != vids_size_prev) {
					ofs << boost::format("%d %d %d %d\n")
						% vids.size() % t_cnt % uids.size() % topics.size();
					vids_size_prev = vids.size();
				}
			}
		}
		if (vids.size() != vids_size_prev) {
			ofs << boost::format("%d %d %d %d\n")
				% vids.size() % t_cnt % uids.size() % topics.size();
		}
		cout << "  Created " << fn << "\n";
	}

	void GetNumUniqAttrGrowthByDCs() {
		Util::CpuTimer _("Gen uniq attrs growth by DCs ...\n");

		map<DC*, set<long> > dc_uids;
		map<DC*, set<string> > dc_vids;
		map<DC*, set<string> > dc_topics;
		set<string> vids;
		size_t vids_size_prev = 0;

		const string& fn = Conf::fn_num_uniq_attrs_growth_by_dcs;
		ofstream ofs(fn);
		if (! ofs.is_open())
			throw runtime_error(str(boost::format("unable to open file %1%") % fn));
		ofs << "# dc vids uids topics\n";

		for (auto uid: _uids) {
			auto it = _users_tweets.find(uid);
			if (it == _users_tweets.end())
				continue;

			for (auto t: it->second) {
				DC* dc = t->LocalDC();
				{
					auto it = dc_uids.find(dc);
					if (it == dc_uids.end()) {
						set<long> s;
						s.insert(uid);
						dc_uids[dc] = s;
					} else {
						it->second.insert(uid);
					}
				}
				{
					auto it = dc_vids.find(dc);
					if (it == dc_vids.end()) {
						set<string> s;
						s.insert(t->youtube_video_id);
						dc_vids[dc] = s;
					} else {
						it->second.insert(t->youtube_video_id);
					}

					vids.insert(t->youtube_video_id);
				}
				{
					auto it = dc_topics.find(dc);
					if (it == dc_topics.end()) {
						set<string> s;
						s.insert(t->topics.begin(), t->topics.end());;
						dc_topics[dc] = s;
					} else {
						it->second.insert(t->topics.begin(), t->topics.end());;
					}
				}

				if ((vids.size() % 100 == 0) && vids.size() != vids_size_prev) {
					for (DC* dc: DCs::GetAll()) {
						ofs << dc->name;
						{
							auto it = dc_vids.find(dc);
							if (it == dc_vids.end()) {
								ofs << " 0";
							} else {
								ofs << " " << it->second.size();
							}
						}
						{
							auto it = dc_uids.find(dc);
							if (it == dc_uids.end()) {
								ofs << " 0";
							} else {
								ofs << " " << it->second.size();
							}
						}
						{
							auto it = dc_topics.find(dc);
							if (it == dc_topics.end()) {
								ofs << " 0";
							} else {
								ofs << " " << it->second.size();
							}
						}
						ofs << "\n";
					}
					vids_size_prev = vids.size();
				}
			}
		}
		if (vids.size() != vids_size_prev) {
			for (DC* dc: DCs::GetAll()) {
				ofs << dc->name;
				{
					auto it = dc_vids.find(dc);
					if (it == dc_vids.end()) {
						ofs << " 0";
					} else {
						ofs << " " << it->second.size();
					}
				}
				{
					auto it = dc_uids.find(dc);
					if (it == dc_uids.end()) {
						ofs << " 0";
					} else {
						ofs << " " << it->second.size();
					}
				}
				{
					auto it = dc_topics.find(dc);
					if (it == dc_topics.end()) {
						ofs << " 0";
					} else {
						ofs << " " << it->second.size();
					}
				}
				ofs << "\n";
			}
		}
		cout << "  Created " << fn << "\n";
	}

	void FreeMem() {
		for (auto ut: _users_tweets)
			for (auto t: ut.second)
				delete t;
	}
}
