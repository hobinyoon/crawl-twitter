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

  void Gen() {
    vector<Tweet*> tweets;
    {
      const string fn = Conf::GetFn("fn_tweets");
      Cons::MT _(str(boost::format("Reading file %s") % fn));
      ifstream ifs(fn, ios::binary);
      size_t num_tweets;
      ifs.read((char*)&num_tweets, sizeof(num_tweets));
      for (size_t i = 0; i < num_tweets; i ++) {
        tweets.push_back(new Tweet(ifs));
      }
      Cons::P(boost::format("%d tweets") % tweets.size());
    }

    set<long> uids;
    {
      for (Tweet* t: tweets)
        uids.insert(t->uid);
      Cons::P(boost::format("%d uniq users") % uids.size());
    }

    {
      Cons::MT _("Writing users to file ...");
      const string dn = Conf::GetFn("out_dn");
      boost::filesystem::create_directories(dn);

      const string fn = str(boost::format("%s/%s") % dn % Conf::GetStr("out_fn"));
      ofstream ofs(fn);
      if (! ofs.is_open())
        throw runtime_error(str(boost::format("unable to open file %1%") % fn));
      int i = 0;
      for (auto uid: uids) {
        i ++;
        if (i % 500 == 1)
          ofs << "insert ignore into users (id, gen, status) values (" << uid << ", -1, 'U')";
        else
          ofs << ", (" << uid << ", -1, 'U')";
        if (i % 500 == 0)
          ofs << ";\n";
      }
      if (i % 500 != 0)
        ofs << ";\n";

      ofs.close();
      Cons::P(boost::format("Generated file %s %d") % fn % boost::filesystem::file_size(fn));
    }
  }

  void Cleanup() {
    for (auto e: _tweets)
      delete e;
  }
}
