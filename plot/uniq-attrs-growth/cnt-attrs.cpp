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

	// get both the number of unique attrs by DCs and the growth of them by DCs
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

		{
			const string& fn = Conf::fn_num_uniq_attrs_by_dcs;
			ofstream ofs(fn);
			if (! ofs.is_open())
				throw runtime_error(str(boost::format("unable to open file %1%") % fn));
			ofs << "# dc       vids  uids  topics\n";
			long sum_vids = 0;
			long sum_uids = 0;
			long sum_topics = 0;
			for (DC* dc: DCs::GetAll()) {
				ofs << boost::format("%-10s %d %d %d\n")
					% dc->name
					% dc_vids[dc].size()
					% dc_uids[dc].size()
					% dc_topics[dc].size();
				sum_vids += dc_vids[dc].size();
				sum_uids += dc_uids[dc].size();
				sum_topics += dc_topics[dc].size();
			}
			size_t num_dcs = DCs::GetAll().size();
			ofs << boost::format("%-10s %.0f %.0f %.0f\n")
				% "Avg"
				% (double(sum_vids) / num_dcs)
				% (double(sum_uids) / num_dcs)
				% (double(sum_topics) / num_dcs);
			cout << "  Created " << fn << "\n";
		}
	}

	void FreeMem() {
		for (auto ut: _users_tweets)
			for (auto t: ut.second)
				delete t;
	}
}
