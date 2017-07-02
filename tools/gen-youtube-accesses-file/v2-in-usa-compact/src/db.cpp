#include <fstream>

#include <boost/algorithm/string.hpp>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include "conf.h"
#include "cons.h"
#include "db.h"
#include "util.h"

using namespace std;

namespace DB {
  sql::Connection* _conn = NULL;
  sql::Statement* _stmt = NULL;

  const string _db_host = "localhost";
  const string _db_user = "twitter";
  const string _db_name = "twitter4";
  string _db_password;

  void _ReadPasswod();

  void Init() {
    _ReadPasswod();

    _conn = get_driver_instance()->
      connect("tcp://" + _db_host + ":3306",
          _db_user,
          _db_password);
    _conn->setSchema(_db_name);
    _stmt = _conn->createStatement();
  }

  void GetAllTweets(vector<Tweet*>& tweets) {
    unique_ptr<sql::ResultSet> rs(_stmt->executeQuery(
          "SELECT id, uid, created_at, geo_lati, geo_longi, youtube_video_id FROM tweets"
          " ORDER BY created_at"
          // Useful for dev
          //" LIMIT 10"
          ));

    int cnt = 0;
    while (rs->next()) {
      tweets.push_back(new Tweet(rs->getInt64("id"),
            rs->getInt64("uid"),
            rs->getString("created_at"),
            rs->getDouble("geo_lati"),
            rs->getDouble("geo_longi"),
            rs->getString("youtube_video_id")
            ));
      cnt ++;
      if (cnt % 10000 == 0) {
        Cons::ClearLine();
        Cons::Pnnl(cnt);
      }
    }
    Cons::ClearLine();
  }

  void Cleanup() {
    delete _conn;
    delete _stmt;
  }

  void _ReadPasswod() {
    string fn = str(boost::format("%s/.my.cnf") % Util::HomeDir());
    ifstream ifs(fn);

    string line;
    while (getline(ifs, line)) {
      boost::trim_right(line);
      //Cons::P(line);
      vector<string> t;
      boost::split(t, line, boost::is_any_of("="));

      if (t.size() != 2)
        continue;

      if (t[0] != "password")
        continue;

      _db_password = t[1];
      return;
    }

    THROW("Unexpected");
  }
};
