#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include "conf.h"
#include "db.h"
#include "tweet.h"
#include "util.h"

using namespace std;

namespace DB {
	sql::Connection* _conn = NULL;
	sql::Statement* _stmt = NULL;

	void Init() {
		_conn = get_driver_instance()->
				connect("tcp://" + Conf::db_host + ":3306", Conf::db_user, Conf::db_pass);
		_conn->setSchema(Conf::db_name);
		_stmt = _conn->createStatement();
	}

	void GetUsersByCrawledAt(vector<long>& uids_by_crawled_at) {
		Util::CpuTimer _("Getting users ...\n", 2);
		unique_ptr<sql::ResultSet> rs(_stmt->executeQuery(
					"SELECT id FROM users WHERE status='C' ORDER BY crawled_at"));
		int i = 0;
		while (rs->next()) {
			uids_by_crawled_at.push_back(rs->getInt64("id"));
			i ++;
			if (i % 1000 == 0) {
				Util::ClearLine();
				cout << "    " << i << flush;
			}
		}
		Util::ClearLine();
		cout << "    uids_by_crawled_at.size()=" << uids_by_crawled_at.size() << "\n";
	}

	void GetTweets(map<long, vector<Tweet*> >& users_tweets) {
		Util::CpuTimer _("Getting tweets ...\n", 2);
		unique_ptr<sql::ResultSet> rs(_stmt->executeQuery(
					"SELECT id, uid, created_at, geo_lati, geo_longi, youtube_video_id, hashtags FROM tweets "));
		int i = 0;
		while (rs->next()) {
			long uid = rs->getInt64("uid");
			Tweet* u = new Tweet(
						rs->getInt64("id"),
						uid,
						rs->getString("created_at"),
						rs->getDouble("geo_lati"),
						rs->getDouble("geo_longi"),
						rs->getString("youtube_video_id"),
						rs->getString("hashtags"));
			auto it = users_tweets.find(uid);
			if (it == users_tweets.end()) {
				vector<Tweet*> users;
				users.push_back(u);
				users_tweets[uid] = users;
			} else {
				it->second.push_back(u);
			}

			i ++;
			if (i % 1000 == 0) {
				Util::ClearLine();
				cout << "    " << i << flush;
			}
		}
		Util::ClearLine();
		cout << "    users_tweets.size()=" << users_tweets.size() << ". " << i << " tweets.\n";
	}

	void Cleanup() {
		delete _conn;
		delete _stmt;
	}
};
