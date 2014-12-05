#include <fstream>
#include <set>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include "conf.h"
#include "tweet.h"
#include "stat.h"
#include "util.h"
#include "youtube-dataset.h"

using namespace std;

namespace YoutubeDataset {
	vector<Tweet*> _tweets;

	void _LoadTweets() {
		const string& fn = Conf::fn_tweets;
		Util::CpuTimer _(str(boost::format("Loading tweets from file %s ...\n") % fn));

		ifstream ifs(fn, ios::binary);
		if (! ifs.is_open())
			throw runtime_error(str(boost::format("unable to open file %1%") % fn));

		size_t e_size;
		ifs.read((char*)&e_size, sizeof(e_size));
		for (size_t i = 0; i < e_size; i ++) {
			_tweets.push_back(new Tweet(ifs));

			if (Conf::partial_load > 0) {
				if (100.0 * i / e_size > Conf::partial_load)
					break;
			}
		}
		cout << "  _tweets.size()=" << _tweets.size() << "\n";
	}

	void Load() {
		_LoadTweets();
	}

	double _CircleSize(double num) {
		return pow(num / 600, (1.0/3));
	}

	void _StatLocs() {
		// rounded coord
		struct RoCoord {
			double longi;
			double lati;

			RoCoord(double lo, double la)
			//: longi(round(lo*2)/2), lati(round(la*2)/2)
			: longi(round(lo/5)*5), lati(round(la/5)*5)
			{ }

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
		for (auto t: _tweets) {
			RoCoord rc(t->geo_longi, t->geo_lati);
			Coord c(t->geo_longi, t->geo_lati);

			auto it = coord_group.find(rc);
			if (it == coord_group.end()) {
				vector<Coord> vc;
				vc.push_back(c);
				coord_group[rc] = vc;
			} else {
				it->second.push_back(c);
			}
		}

		const string& fn = Conf::fn_result;
		ofstream ofs(fn);
		if (! ofs.is_open())
			throw runtime_error(str(boost::format("unable to open file %s") % fn));
		for (auto cg: coord_group) {
			double lo = 0.0;
			double la = 0.0;
			for (auto& c: cg.second) {
				lo += c.longi;
				la += c.lati;
			}
			lo /= cg.second.size();
			la /= cg.second.size();

			ofs << boost::format("%10.3f %10.3f %7d %8f\n")
				% lo % la % cg.second.size() % _CircleSize(cg.second.size());
		}

		// legends circles and labels
		int lcs[] = {10, 100, 1000, 10000, 100000};
		const char* lcsl[] = {"10^1", "10^2", "10^3", "10^4", "10^5"};
		size_t lc_len = sizeof(lcs) / sizeof(int);
		for (size_t i = 0; i < lc_len; i ++) {
			ofs << boost::format("%10.3f %10.3f %7d %8f %s\n")
				% (-30.0 + 15.0 * i)
				% -55.0
				% lcs[i]
				% _CircleSize(lcs[i])
				% lcsl[i];
		}

		ofs.close();
		cout << "  Generated file " << fn << " size=" << boost::filesystem::file_size(fn) << "\n";
	}

	struct Loc {
		double longi;
		double lati;

		Loc() {
		}

		Loc(double lo, double la)
			: longi(lo), lati(la)
		{}
	};

	double _Dist(const Loc& a, const Loc& b) {
		double c = a.longi - b.longi;
		double d = a.lati - b.lati;
		return sqrt(c*c + d*d);
	}

	void GenStat() {
		Util::CpuTimer _("Generating stats ...\n");

		_StatLocs();
	}

	void FreeMem() {
		Util::CpuTimer _("Freeing memory ...\n");

		for (auto& r: _tweets)
			delete r;
		_tweets.clear();
	}
};
