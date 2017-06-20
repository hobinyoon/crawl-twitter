#include <fstream>
#include <set>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include "conf.h"
#include "cons.h"
#include "op-youtube.h"
#include "stat.h"
#include "util.h"
#include "youtube-dataset.h"

using namespace std;

string _ToCompactDatetime(boost::posix_time::ptime& ptime) {
	// http://stackoverflow.com/questions/5018188/how-to-format-a-datetime-to-string-using-boost
	// Parsing strings directly would be faster
	static locale loc(cout.getloc(), new boost::posix_time::time_facet("%y%m%d-%H%M%S"));
	basic_stringstream<char> ws;
	ws.imbue(loc);
	ws << ptime;
	return ws.str();
}


namespace YoutubeDataset {
	vector<OpYoutube*> _ops;

	void _LoadOps() {
		const string& fn = Conf::GetFn("fn_tweets");
		Cons::MT _(str(boost::format("Loading YouTube access locations from file %s ...") % fn));

		ifstream ifs(fn, ios::binary);
		if (! ifs.is_open())
			throw runtime_error(str(boost::format("unable to open file %1%") % fn));

		size_t e_size;
		ifs.read((char*)&e_size, sizeof(e_size));
		for (size_t i = 0; i < e_size; i ++) {
			_ops.push_back(new OpYoutube(ifs));
		}
		cout << "  _ops.size()=" << _ops.size() << "\n";
	}


	void Load() {
		_LoadOps();
	}


#if 0
	void _TimeNumTweetsCDF() {
		Cons::MT _("Generating stats ...\n");

		const string& fn = "num-tweets-by-time";
		ofstream ofs(fn);
		if (! ofs.is_open())
			throw runtime_error(str(boost::format("unable to open file %s") % fn));

		double prev_y = -1.0;
		double y = 0.0;
		int i = 0;
		bool last_one_printed = false;
		OpYoutube* last_tweet = NULL;

		if (_ops.size() == 0)
			throw runtime_error(str(boost::format("unexpected _ops.size()=%d") % _ops.size()));

		boost::posix_time::ptime* ca_begin = &((*_ops.begin())->created_at_pt);
		boost::posix_time::ptime* ca_end = &((*_ops.rbegin())->created_at_pt);
		//boost::posix_time::time_duration
		double ca_dur_secs = (*ca_end - *ca_begin).total_seconds();

		//boost::posix_time::ptime* ca_prev = NULL;
		for (auto& t: _ops) {
			i ++;

			last_one_printed = false;
			last_tweet = t;
			y = double(i) / _ops.size();
			if (y - prev_y >= 0.001) {
				ofs << boost::format("%s %f %f\n")
					% _ToCompactDatetime(t->created_at_pt)
					% ((t->created_at_pt - *ca_begin).total_seconds() / ca_dur_secs)
					% y;
				prev_y = y;
				//ca_prev = &(t->created_at_pt);
				last_one_printed = true;
			}
			// If needed,
			//else if (x - prev_x has big enough interval) {
			// do the same.
			//}
		}
		if (! last_one_printed) {
			y = double(i) / _ops.size();
			ofs << boost::format("%s %f %f\n")
				% _ToCompactDatetime(last_tweet->created_at_pt)
				% ((last_tweet->created_at_pt - *ca_begin).total_seconds() / ca_dur_secs)
				% y;
		}

		ofs.close();
		cout << "  Generated file " << fn << " size=" << boost::filesystem::file_size(fn) << "\n";
	}
#endif


	void _NumTweetsByDay() {
		Cons::MT _("Generating stats by months ...");

		map<string, int> day_cnt;
		for (auto& t: _ops) {
			// 2010-08-12 12:27:39
			// 0123456789
			//cout << boost::format("%s\n") % t->created_at.substr(0, 7);
			string day = t->created_at.substr(0, 10);
			auto i = day_cnt.find(day);
			if (i == day_cnt.end())
				day_cnt.emplace(day, 1);
			else
				i->second ++;
		}

		string dn = Conf::GetFn("out_dir");
		boost::filesystem::create_directory(dn);

		string out_fn = str(boost::format("%s/videoreqs-by-day") % dn);
		ofstream ofs(out_fn);
		if (! ofs.is_open())
			throw runtime_error(str(boost::format("unable to open file %s") % out_fn));
		for (const auto e: day_cnt)
			ofs << boost::format("%s %6d\n") % e.first % e.second;
		ofs.close();
		Cons::P(boost::format("Created %s %d") % out_fn % boost::filesystem::file_size(out_fn));
	}


	void GenStat() {
		//_TimeNumTweetsCDF();
		_NumTweetsByDay();
	}


	void FreeMem() {
		Cons::MT _("Freeing memory ...");

		for (auto& r: _ops)
			delete r;
		_ops.clear();
	}
};
