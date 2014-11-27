#include <fstream>
#include <set>
#include <boost/filesystem.hpp>
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

	void CntNumUniqAttrGrowth() {
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

		ofs << Util::Prepend(Conf::Desc(), "# ");
		ofs << "#\n";
		ofs << "#  vids  tweets            uids         topics\n";

		size_t vids_size_prev = 0;

		for (auto uid: _uids) {
			auto it = _users_tweets.find(uid);
			if (it == _users_tweets.end())
				continue;

			for (auto t: it->second) {
				if (t->type == Tweet::Type::W)
					continue;

				uids.insert(uid);
				vids.insert(t->youtube_video_id);
				topics.insert(t->topics.begin(), t->topics.end());
				t_cnt ++;

				if ((vids.size() % 100 == 0) && vids.size() != vids_size_prev) {
					ofs << boost::format("%7d"
							" %7d %5.2f%%"
							" %7d %5.2f%%"
							" %7d %5.2f%%\n")
						% vids.size()
						% t_cnt % (100.0 * t_cnt / vids.size())
						% uids.size() % (100.0 * uids.size() / vids.size())
						% topics.size() % (100.0 * topics.size() / vids.size());
					vids_size_prev = vids.size();
				}
			}
		}
		if (vids.size() != vids_size_prev) {
			ofs << boost::format("%7d"
					" %7d %5.2f%%"
					" %7d %5.2f%%"
					" %7d %5.2f%%\n")
				% vids.size()
				% t_cnt % (100.0 * t_cnt / vids.size())
				% uids.size() % (100.0 * uids.size() / vids.size())
				% topics.size() % (100.0 * topics.size() / vids.size());
		}
		cout << "  Created " << fn << " " << boost::filesystem::file_size(fn) << "\n";
	}


	void CntNumUniqAttrGrowthByDCs() {
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
		ofs << "# dc         vids   uids topics\n";

		for (auto uid: _uids) {
			auto it = _users_tweets.find(uid);
			if (it == _users_tweets.end())
				continue;

			for (auto t: it->second) {
				if (t->type == Tweet::Type::W)
					continue;

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
						size_t num_vids = 0;
						size_t num_uids = 0;
						size_t num_topics = 0;
						{
							auto it = dc_vids.find(dc);
							if (it != dc_vids.end())
								num_vids = it->second.size();
						}
						{
							auto it = dc_uids.find(dc);
							if (it != dc_uids.end())
								num_uids = it->second.size();
						}
						{
							auto it = dc_topics.find(dc);
							if (it != dc_topics.end())
								num_topics = it->second.size();
						}
						ofs << boost::format("%-10s %6d %6d %6d\n")
							% dc->name
							% num_vids
							% num_uids
							% num_topics;
					}
					vids_size_prev = vids.size();
				}
			}
		}
		if (vids.size() != vids_size_prev) {
			for (DC* dc: DCs::GetAll()) {
				size_t num_vids = 0;
				size_t num_uids = 0;
				size_t num_topics = 0;
				{
					auto it = dc_vids.find(dc);
					if (it != dc_vids.end())
						num_vids = it->second.size();
				}
				{
					auto it = dc_uids.find(dc);
					if (it != dc_uids.end())
						num_uids = it->second.size();
				}
				{
					auto it = dc_topics.find(dc);
					if (it != dc_topics.end())
						num_topics = it->second.size();
				}
				ofs << boost::format("%-10s %6d %6d %6d\n")
					% dc->name
					% num_vids
					% num_uids
					% num_topics;
			}
		}
		cout << "  Created " << fn << " " << boost::filesystem::file_size(fn) << "\n";

		//{
		//	const string& fn = Conf::fn_num_uniq_attrs_by_dcs;
		//	ofstream ofs(fn);
		//	if (! ofs.is_open())
		//		throw runtime_error(str(boost::format("unable to open file %1%") % fn));
		//	ofs << "# dc       vids  uids  topics\n";
		//	long sum_vids = 0;
		//	long sum_uids = 0;
		//	long sum_topics = 0;
		//	for (DC* dc: DCs::GetAll()) {
		//		ofs << boost::format("%-10s %d %d %d\n")
		//			% dc->name
		//			% dc_vids[dc].size()
		//			% dc_uids[dc].size()
		//			% dc_topics[dc].size();
		//		sum_vids += dc_vids[dc].size();
		//		sum_uids += dc_uids[dc].size();
		//		sum_topics += dc_topics[dc].size();
		//	}
		//	size_t num_dcs = DCs::GetAll().size();
		//	ofs << boost::format("%-10s %.0f %.0f %.0f\n")
		//		% "Avg"
		//		% (double(sum_vids) / num_dcs)
		//		% (double(sum_uids) / num_dcs)
		//		% (double(sum_topics) / num_dcs);
		//	cout << "  Created " << fn << " " << boost::filesystem::file_size(fn) << "\n";
		//}
	}

	void FreeMem() {
		for (auto ut: _users_tweets)
			for (auto t: ut.second)
				delete t;
	}
}
