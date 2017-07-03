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
    vector<long> uids;
    {
      Cons::MT _("Loading distict users from the tweet table ...");
      DB::GetUniqUsersFromCrawledTweets(uids);
      Cons::P(boost::format("%d users") % uids.size());
    }

    {
      Cons::MT _("Writing users to file ...");
      const string fn = "patch.sql";
      ofstream ofs(fn);
      if (! ofs.is_open())
        throw runtime_error(str(boost::format("unable to open file %1%") % fn));
      int i = 0;
      for (auto uid: uids) {
        i ++;
        if (i % 500 == 1)
          ofs << "update users set status='U', gen=-1 where id IN (" << uid;
        else
          ofs << ", " << uid;
        if (i % 500 == 0)
          ofs << ");\n";
      }
      if (i % 500 != 0)
        ofs << ");\n";

      ofs.close();
      Cons::P(boost::format("Generated file %s %d") % fn % boost::filesystem::file_size(fn));
    }
  }

  void Cleanup() {
    for (auto e: _tweets)
      delete e;
  }
}
