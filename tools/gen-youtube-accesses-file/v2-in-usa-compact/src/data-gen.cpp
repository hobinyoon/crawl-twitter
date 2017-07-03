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

  void _LoadTweetsFromDB() {
    Cons::MT _("Loading tweets from DB ...");
    DB::GetAllTweets(_tweets);
    Cons::P(boost::format("%d tweets") % _tweets.size());
  }

  void _WriteTweetsToFile() {
    Cons::MT _("Writing tweets to file ...");
    size_t e_size = _tweets.size();
    const string fn = str(boost::format("%s/%s-tweets-%d")
        % Conf::GetFn("out_dn") % _cur_datetime % e_size);
    ofstream ofs(fn, ios::binary);
    if (! ofs.is_open())
      throw runtime_error(str(boost::format("unable to open file %1%") % fn));
    ofs.write((char*)&e_size, sizeof(size_t));
    for (auto o: _tweets)
      o->Write(ofs);
    ofs.close();
    Cons::P(boost::format("Generated file %s %d") % fn % boost::filesystem::file_size(fn));
  }


  // Filter out excessive Tweets with the same video from the same person in a short period of time.
  void _FilterOutAds() {
    Cons::MT _("Filtering out ads ...");

    // At most 1 Tweet per video per person per location in the last 12 hours.
    const long min_time_interval = 6 * 3600;

    struct VidUidLoc {
      string vid;
      long uid;
      double lon;
      double lat;

      VidUidLoc(const string vid_, const long uid_, double lon_, double lat_)
        : vid(vid_), uid(uid_), lon(lon_), lat(lat_)
      {}

      bool operator < (const VidUidLoc& r) const {
        if (vid < r.vid) return true;
        if (vid > r.vid) return false;

        if (uid < r.uid) return true;
        if (uid > r.uid) return false;

        if (lon < r.lon) return true;
        if (lon > r.lon) return false;

        return (lat < r.lat);
      }
    };

    map<VidUidLoc, boost::posix_time::ptime> key_lasttime;

    vector<Tweet*> new_tweets;
    int num_filtered_out = 0;
    size_t size_before = _tweets.size();

    for (Tweet* t: _tweets) {
      const string& vid = t->youtube_video_id;
      long uid = t->uid;
      // created at
      const boost::posix_time::ptime& ca = boost::posix_time::time_from_string(t->created_at);

      VidUidLoc k(vid, uid, t->geo_longi, t->geo_lati);
      auto it = key_lasttime.find(k);
      if (it == key_lasttime.end()) {
        key_lasttime[k] = ca;
        new_tweets.push_back(t);
      } else {
        const boost::posix_time::ptime& last_ca = key_lasttime[k];
        if ((ca - last_ca).total_seconds() < min_time_interval) {
          // Ignore Tweets that are too close to the previous one
          delete t;
          num_filtered_out ++;
        } else {
          key_lasttime[k] = ca;
          new_tweets.push_back(t);
        }
      }
    }

    _tweets = new_tweets;
    size_t size_after = _tweets.size();

    Cons::P(boost::format("%d -> %d %.2f%% reduction")
        % size_before
        % size_after
        % (100.0 * (size_before - size_after) / size_before)
        );
  }

  void Gen() {
    _LoadTweetsFromDB();
    _FilterOutAds();
    _WriteTweetsToFile();
  }

  void Cleanup() {
    for (auto e: _tweets)
      delete e;
  }
}
