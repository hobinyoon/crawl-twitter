#include <fstream>
#include <set>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include "conf.h"
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
		const string& fn = Conf::fn_tweets;
		Util::CpuTimer _(str(boost::format("Loading tweets from file %s ...\n") % fn));

		ifstream ifs(fn, ios::binary);
		if (! ifs.is_open())
			throw runtime_error(str(boost::format("unable to open file %1%") % fn));

		size_t e_size;
		ifs.read((char*)&e_size, sizeof(e_size));
		for (size_t i = 0; i < e_size; i ++)
			_tweets.push_back(new Tweet(ifs));
		cout << "  _tweets.size()=" << _tweets.size() << "\n";
	}


	void Load() {
		_LoadTweets();
	}


	void GenStat() {
		Util::CpuTimer _("Generating stats ...\n");

		const string& fn = Conf::fn_output;
		ofstream ofs(fn);
		if (! ofs.is_open())
			throw runtime_error(str(boost::format("unable to open file %s") % fn));

		double prev_y = -1.0;
		double y = 0.0;
		int i = 0;
		bool last_one_printed = false;
		Tweet* last_tweet = NULL;
		for (auto t: _tweets) {
			i ++;
			last_one_printed = false;
			last_tweet = t;
			y = double(i) / _tweets.size();
			if (y - prev_y >= 0.001) {
				ofs << boost::format("%s %f\n") % _ToCompactDatetime(t->created_at) % y;
				prev_y = y;
				last_one_printed = true;
			}
			// If needed,
			//else if (x - prev_x has big enough interval) {
			// do the same.
			//}
		}
		if (! last_one_printed) {
			y = double(i) / _tweets.size();
			ofs << boost::format("%s %f\n") % _ToCompactDatetime(last_tweet->created_at) % y;
		}

		ofs.close();
		cout << "  Generated file " << fn << " size=" << boost::filesystem::file_size(fn) << "\n";
	}


	void FreeMem() {
		Util::CpuTimer _("Freeing memory ...\n");

		for (auto& r: _tweets)
			delete r;
		_tweets.clear();
	}
};
