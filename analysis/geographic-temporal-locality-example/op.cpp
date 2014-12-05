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

	// map<DC*, map<time, cnt> >
	map<DC*, map<string, int> > _dc_time_cnt;

	void _PrintIBCByDCsByTime() {
		boost::posix_time::ptime ca_b = boost::posix_time::time_from_string("2014-06-29 00:00:00");
		boost::posix_time::ptime ca_e = boost::posix_time::time_from_string("2014-08-31 00:00:00");

		stringstream ss;

		ss << "#        ";
		for (auto dtc: _dc_time_cnt)
			ss << boost::format(" %10s") % dtc.first->name;
		ss << boost::format(" %10s") % "Sum";
		ss << "\n";

		for (boost::posix_time::ptime ca = ca_b; ca <= ca_e; ca += boost::gregorian::days(1)) {
			//ss << ca << "\n";
			tm tm_ = to_tm(ca);
			string date = str(boost::format("%d-%02d-%02d") % (tm_.tm_year + 1900) % (tm_.tm_mon + 1) % tm_.tm_mday);
			//ss << date << "\n";

			ss << boost::format("%10s") % date;
			int sum = 0;
			for (auto dtc: _dc_time_cnt) {
				int num = 0;
				auto it = dtc.second.find(date);
				if (it != dtc.second.end())
					num = it->second;
				sum += num;
				ss << boost::format(" %10d") % num;
			}
			ss << boost::format(" %10d") % sum;
			ss << "\n";
		}

		const string& fn = Conf::fn_ibc_by_time_by_dcs;
		ofstream ofs(fn);
		if (! ofs.is_open())
			throw runtime_error(str(boost::format("Unable to open file %1%") % fn));
		ofs << ss.str();
		cout << "  Created file " << fn << "\n";
	}

	void CntIBCByDCsByTime() {
		Util::CpuTimer _("Counting icebucketchallenge by DCs by time ...\n");

		for (auto& e: _entries) {
			DC* dc = DCs::GetClosest(e->geo_lati, e->geo_longi);

			auto it = _dc_time_cnt.find(dc);
			if (it == _dc_time_cnt.end()) {
				map<string, int> m;
				for (auto& t: e->topics) {
					if (t.find("icebucketchallenge") == string::npos
							&& t.find("ibc") == string::npos )
						continue;
					m[e->created_at_str.substr(0, 10)] = 1;
				}
				_dc_time_cnt[dc] = m;
			} else {
				map<string, int>& m = it->second;
				for (auto& t: e->topics) {
					if (t.find("icebucketchallenge") == string::npos
							&& t.find("ibc") == string::npos )
						continue;
					string key = e->created_at_str.substr(0, 10);
					auto it2 = m.find(key);
					if (it2 == m.end()) {
						m[key] = 1;
					} else {
						it2->second ++;
					}
				}
			}
		}

		_PrintIBCByDCsByTime();
	}

	double _CircleSize(int i) {
		return pow(i / 20.0, (1.0/3));
	}


	void CntFCKByDCsByLoc() {
		Util::CpuTimer _("Counting icebucketchallenge by locations ...\n");

		// rounded coord
		struct RoCoord {
			double longi;
			double lati;

			RoCoord(double lo, double la)
			// : longi(round(lo/5)*5), lati(round(la/5)*5)
			{
				longi = nearbyint(lo);
				lati = nearbyint(la);
			}

			bool operator < (const RoCoord& rhs) const {
				if (longi < rhs.longi) return true;
				if (longi > rhs.longi) return false;
				return (lati < rhs.lati);
			}
		};

		struct Coord {
			double longi;
			double lati;

			Coord(double lo, double la)
			: longi(lo), lati(la)
			{ }
		};

		map<RoCoord, vector<Coord> > coord_group;
		for (auto& e: _entries) {
			//if (e->created_at_str < "2014-08-13 00:00:00")
			//	continue;
			if (e->created_at_str.substr(0, 4) != "2013")
				continue;

			for (auto& t: e->topics) {
				//if (t.find("icebucketchallenge") == string::npos
				//		&& t.find("ibc") == string::npos )
				//	continue;
				//if (t.find("fallontonight") == string::npos)
				//	continue;
				//if (t != "lfc")
				//	continue;
				//if (t != "tomorrowland")
				//	continue;

				// mysql> select created_at, geo_lati, geo_longi, hashtags from tweets where hashtags REGEXP '[[:<:]]fck[[:>:]]';
				if (t == "fck") {
					RoCoord rc(e->geo_longi, e->geo_lati);
					Coord c(e->geo_longi, e->geo_lati);

					auto it = coord_group.find(rc);
					if (it == coord_group.end()) {
						vector<Coord> vc;
						vc.push_back(c);
						coord_group[rc] = vc;
					} else {
						it->second.push_back(c);
					}
					continue;
				}
			}
		}

		const string& fn = Conf::fn_fck_by_loc;
		ofstream ofs(fn);
		if (! ofs.is_open())
			throw runtime_error(str(boost::format("Unable to open file %1%") % fn));
		for (auto cg: coord_group) {

			double lo = 0.0;
			double la = 0.0;
			for (auto& c: cg.second) {
				lo += c.longi;
				la += c.lati;
			}
			lo /= cg.second.size();
			la /= cg.second.size();

			ofs << boost::format("%5.1f %5.1f %f %d\n")
				% lo % la % _CircleSize(cg.second.size()) % cg.second.size();
		}
		int sizes[] = {1, 10, 20, 30};
		size_t sizes_len = sizeof(sizes) / sizeof(int);
		for (size_t i = 0; i < sizes_len; i ++) {
			double longi = 5.5 + 3.0 * i;
			double lati = 33.0;
			ofs << boost::format("%5.1f %5.1f %f %d %d\n")
				% longi % lati % _CircleSize(sizes[i]) % sizes[i] % sizes[i];
		}

		cout << "  Created file " << fn << "\n";
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
