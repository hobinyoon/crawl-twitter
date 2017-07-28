#include <ctime>
#include <fstream>
#include <set>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include "conf.h"
#include "db.h"
#include "data-gen.h"
#include "tweet.h"
#include "util.h"

using namespace std;

namespace DataGen {
  string _cur_datetime;
  vector<Tweet*> _tweets;


  void _SetCurDatetime() {
    time_t t = time(NULL);
    struct tm* now = localtime(&t);
    _cur_datetime = str(boost::format("%02d%02d%02d-%02d%02d%02d")
        % (now->tm_year + 1900 - 2000)
        % (now->tm_mon + 1)
        % now->tm_mday
        % now->tm_hour
        % now->tm_min
        % now->tm_sec
        );
    Cons::P(boost::format("_cur_datetime=%s") % _cur_datetime);
  }

  void Init() {
    _SetCurDatetime();
  }

  void _ReadFromFile() {
    // Parameterize when needed
    const string fn = "/home/hobin/work/cp-mec-data/170728-103709-tweets-975516";
    Cons::MT _(boost::format("Reading file %s ...") % fn);
		ifstream ifs(fn, ios::binary);
		if (! ifs.is_open())
			THROW(boost::format("unable to open file %1%") % fn);

		size_t e_size;
		ifs.read((char*)&e_size, sizeof(e_size));
    Cons::P(e_size);

		for (size_t i = 0; i < e_size; i ++) {
      Tweet* t = new Tweet(ifs);
      _tweets.push_back(t);
      //cout << *t << "\n";
      Cons::P(to_string(*t));
      if (i == 20)
        break;
		}
  }

  void Read() {
    _ReadFromFile();
  }

  void Cleanup() {
    for (auto e: _tweets)
      delete e;
  }
}
