#include <fstream>
#include <set>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include "conf.h"
#include "cons.h"
#include "tweet.h"
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
	vector<Tweet*> _tweets;


	void _LoadTweets() {
		const string& fn = Conf::GetFn("fn_tweets");
		Cons::MT _(str(boost::format("Loading tweets from file %s ...\n") % fn));

		ifstream ifs(fn, ios::binary);
		if (! ifs.is_open())
			throw runtime_error(str(boost::format("unable to open file %1%") % fn));

		size_t e_size;
		ifs.read((char*)&e_size, sizeof(e_size));
		for (size_t i = 0; i < e_size; i ++) {
			_tweets.push_back(new Tweet(ifs));
		}
		cout << "  _tweets.size()=" << _tweets.size() << "\n";
	}


	void Load() {
		_LoadTweets();
	}


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
		Tweet* last_tweet = NULL;

		if (_tweets.size() == 0)
			throw runtime_error(str(boost::format("unexpected _tweets.size()=%d") % _tweets.size()));

		boost::posix_time::ptime* ca_begin = &((*_tweets.begin())->created_at);
		boost::posix_time::ptime* ca_end = &((*_tweets.rbegin())->created_at);
		//boost::posix_time::time_duration
		double ca_dur_secs = (*ca_end - *ca_begin).total_seconds();

		//boost::posix_time::ptime* ca_prev = NULL;
		for (auto& t: _tweets) {
			i ++;

			last_one_printed = false;
			last_tweet = t;
			y = double(i) / _tweets.size();
			if (y - prev_y >= 0.001) {
				ofs << boost::format("%s %f %f\n")
					% _ToCompactDatetime(t->created_at)
					% ((t->created_at - *ca_begin).total_seconds() / ca_dur_secs)
					% y;
				prev_y = y;
				//ca_prev = &(t->created_at);
				last_one_printed = true;
			}
			// If needed,
			//else if (x - prev_x has big enough interval) {
			// do the same.
			//}
		}
		if (! last_one_printed) {
			y = double(i) / _tweets.size();
			ofs << boost::format("%s %f %f\n")
				% _ToCompactDatetime(last_tweet->created_at)
				% ((last_tweet->created_at - *ca_begin).total_seconds() / ca_dur_secs)
				% y;
		}

		ofs.close();
		cout << "  Generated file " << fn << " size=" << boost::filesystem::file_size(fn) << "\n";
	}


	void _MonthNumTweets() {
		Cons::MT _("Generating stats by months ...\n");

		map<string, int> month_cnt;
		for (auto& t: _tweets) {
			// 2010-08-12 12:27:39
			// 0123456
			//cout << boost::format("%s\n") % t->created_at_str.substr(0, 7);
			string month = t->created_at_str.substr(0, 7);
			auto i = month_cnt.find(month);
			if (i == month_cnt.end())
				month_cnt.emplace(month, 1);
			else
				i->second ++;
		}

		for (auto const &e: month_cnt) {
			cout << boost::format("  %s %6d\n") % e.first % e.second;
		}
	}


	void GenStat() {
		//_TimeNumTweetsCDF();
		_MonthNumTweets();
	}


	void FreeMem() {
		Cons::MT _("Freeing memory ...\n");

		for (auto& r: _tweets)
			delete r;
		_tweets.clear();
	}
};
