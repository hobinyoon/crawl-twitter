#include <fstream>
#include <set>
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

	Entry::Entry(
			const string& created_at_,
			double geo_lati_,
			double geo_longi_)
		: geo_lati(geo_lati_), geo_longi(geo_longi_)
	{
		created_at = boost::posix_time::time_from_string(created_at_);
	}

	static list<Entry*> _entries;

	void _LoadTweetsFromFile() {
		const string& fn = Conf::fn_input;
		Util::CpuTimer _(str(boost::format("Loading tweets from file %s ...\n") % fn), 2);

		ifstream ifs(fn);
		if (! ifs.is_open())
			THROW_RE(boost::format("unable to open file %s") % fn);

		static const auto sep = boost::is_any_of("\t");
		static const auto sep1 = boost::is_any_of(" ");
		int num_dropped_tweets = 0;
		string line;
		while (getline(ifs, line)) {
			//TRACE << line << "\n";
			if (line.size() == 0)
				continue;
			if (line[0] == '#')
				continue;
			vector<string> t;
			boost::split(t, line, sep);
			//TRACE << t.size() << "\n";
			if (t.size() != 11)
				THROW_RE(boost::format("unexpected format [%s]") % line);
			const string& datetime = t[2];
			const string& lati = t[3];
			const string& longi = t[4];
			const string& hashtags = t[6];

			// Filter by year_month_date or month_date
			if (Conf::year_month_date_0.size() > 0 && Conf::year_month_date_1.size() > 0) {
				if (datetime < Conf::year_month_date_0)
					continue;
				if (datetime > Conf::year_month_date_1)
					continue;
			} else if (Conf::month_date_0.size() > 0 && Conf::month_date_1.size() > 0) {
				if (datetime.substr(5) < Conf::month_date_0)
					continue;
				if (datetime.substr(5) > Conf::month_date_1)
					continue;
			}

			vector<string> t1;
			boost::split(t1, hashtags, sep1);
			//TRACE << "\n";
			_entries.push_back(new Entry(datetime, atof(lati.c_str()), atof(longi.c_str())));

			// Filter by exact word match
			//bool topic_matched = false;
			//for (const auto& ht: t1) {
			//	//TRACE << ht << "\n";
			//	if (boost::iequals(ht, Conf::topic)) {
			//		topic_matched = true;
			//		break;
			//	}
			//}
			//if (topic_matched) {
			//	_entries.push_back(new Entry(datetime, atof(lati.c_str()), atof(longi.c_str())));
			//	//TRACE << datetime << "\n";
			//} else {
			//	num_dropped_tweets ++;
			//	//TRACE << hashtags << "\n";
			//}
		}
		cout << boost::format("    _entries.size()=%d dropped %d\n") % _entries.size() % num_dropped_tweets;
	}


	void _ClusterLocs() {
		// rounded coord
		struct RoCoord {
			double longi;
			double lati;

			RoCoord(double lo, double la) {
				longi = round(lo/Conf::cluster_dist) * Conf::cluster_dist;
				lati = round(la/Conf::cluster_dist) * Conf::cluster_dist;
				//TRACE << longi << " " << lati << "\n";
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
		}

		const string& fn = Conf::fn_output;
		ofstream ofs(fn);
		if (! ofs.is_open())
			throw runtime_error(str(boost::format("Unable to open file %1%") % fn));
		ofs << "# longi lati num_tweets\n";

		for (auto cg: coord_group) {
			double lo = 0.0;
			double la = 0.0;
			for (auto& c: cg.second) {
				lo += c.longi;
				la += c.lati;
			}
			lo /= cg.second.size();
			la /= cg.second.size();

			ofs << boost::format("%6.1f %6.1f %3d\n")
				% lo % la % cg.second.size();
		}
		//int sizes[] = {1, 5, 10, 20};
		//size_t sizes_len = sizeof(sizes) / sizeof(int);
		//for (size_t i = 0; i < sizes_len; i ++) {
		//	double longi = -1.0 + 6.0 * i;
		//	double lati = 31.0;
		//	ofs << boost::format("%5.1f %5.1f %f %3d %3d\n")
		//		% longi % lati % _CircleSize(sizes[i]) % sizes[i] % sizes[i];
		//}
		ofs.close();

		cout << "  Created file " << fn << " " << boost::filesystem::file_size(fn) << "\n";
	}


	void Load() {
		Util::CpuTimer _("Loading tweets ...\n");

		_LoadTweetsFromFile();
		_ClusterLocs();
	}


	double _CircleSize(int i) {
		return pow(i / 5.0, (1.0/3));
	}


	void FreeMem() {
		for (auto e: _entries)
			delete e;
	}
}


std::ostream& operator<< (std::ostream& os, const Ops::Entry& op) {
	os << boost::format("%s %10lf %11lf")
		% op.created_at
		% op.geo_lati
		% op.geo_longi;
	return os;
}


//std::ostream& operator<< (std::ostream& os, const Ops::Entry& op) {
//	os << boost::format("%ld %10ld %s %s %10lf %11lf %s")
//		% op.id
//		% op.uid
//		% op.created_at
//		% op.type
//		% op.geo_lati
//		% op.geo_longi
//		% op.youtube_video_id;
//	return os;
//}
