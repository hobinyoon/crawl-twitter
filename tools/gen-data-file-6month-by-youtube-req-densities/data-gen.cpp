#include <ctime>
#include <fstream>
#include <set>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include "conf.h"
#include "data-gen.h"
#include "tweet.h"
#include "util.h"

using namespace std;

namespace DataGen {
	string _cur_datetime;

	sql::Connection* _conn = NULL;
	sql::Statement* _stmt = NULL;


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
		//cout << _cur_datetime << "\n";
	}


	void Init() {
		_SetCurDatetime();

		_conn = get_driver_instance()->
			connect("tcp://" + Conf::db_host + ":3306", Conf::db_user, Conf::db_pass);
		_conn->setSchema(Conf::db_name);
		_stmt = _conn->createStatement();
	}

	list<long> _uids;

	void _LoadUsersFromDB(list<long>& uids) {
		Util::CpuTimer _("Loading users from DB ...\n");
		unique_ptr<sql::ResultSet> rs(_stmt->executeQuery(
					"SELECT id FROM users WHERE status='C' ORDER BY crawled_at"));
		int i = 0;
		while (rs->next()) {
			uids.push_back(rs->getInt64("id"));
			i ++;
			if (i % 10000 == 0) {
				Util::ClearLine();
				cout << "  " << i << flush;
			}
		}
		Util::ClearLine();
		cout << "  uids.size()=" << uids.size() << "\n";
	}


	void _LoadTweetsFromDB(list<Tweet*>& tweets) {
		Util::CpuTimer _("Loading tweets from DB ...\n");
		unique_ptr<sql::ResultSet> rs(_stmt->executeQuery(
					"SELECT id, uid, created_at, geo_lati, geo_longi, youtube_video_id, hashtags FROM tweets "
					" WHERE"
					// Useful during dev
					//" id < 380000000000000000 "
					// Filter by time range
					" '2013-11-01' <= created_at AND created_at < '2014-05-01'"
					" ORDER BY created_at "));

		int cnt = 0;
		while (rs->next()) {
			tweets.push_back(new Tweet(rs->getInt64("id"),
						rs->getInt64("uid"),
						rs->getString("created_at"),
						rs->getDouble("geo_lati"),
						rs->getDouble("geo_longi"),
						rs->getString("youtube_video_id"),
						rs->getString("hashtags")));
			cnt ++;
			if (cnt % 10000 == 0) {
				Util::ClearLine();
				cout << "  " << cnt << flush;
			}
		}
		Util::ClearLine();

		cout << "  tweets.size()=" << tweets.size() << "\n";

		//for (auto t: tweets)
		//	cout << "  " << *t << "\n";
	}

	// Filter out tweets without user. This returns Tweets with various densities
	// over time. This also filters out half-crawled tweets.
	void _FilterOutTweetsWoUser(const list<long>& uids, const list<Tweet*>& tweets_all, list<Tweet*>& tweets) {
		Util::CpuTimer _("Filtering out tweets without user info ...\n");
		size_t before = tweets_all.size();

		set<long> uid_set;
		for (auto uid: uids)
			uid_set.insert(uid);

		for (auto it = tweets_all.begin(); it != tweets_all.end(); it ++) {
			if (uid_set.count((*it)->uid) == 1)
				tweets.push_back(*it);
		}

		cout << boost::format("  before %d, after %d (%.2f%%)\n")
			% before
			% tweets.size() % (100.0 * tweets.size() / before);
	}

	// Set uploader and topics and filter out lonely tweets
	void _SetUploaderTopicsAndFilterOutLonelyTweets(
			const list<Tweet*>& tweets_before
			, list<Tweet*>& tweets_after)
	{
		Util::CpuTimer _("Filtering out lonely tweets and setting uploader and topics ...\n");

		size_t before = tweets_before.size();

		map<string, vector<Tweet*> > by_youtube_video_ids;
		for (auto e: tweets_before) {
			auto it = by_youtube_video_ids.find(e->youtube_video_id);
			if (it == by_youtube_video_ids.end()) {
				vector<Tweet*> v;
				v.push_back(e);
				by_youtube_video_ids[e->youtube_video_id] = v;
			} else {
				it->second.push_back(e);
			}
		}

		{
			Util::CpuTimer _("Setting uploader and topics ...\n", 2);
			// The first one is W and all the others are Rs. For more accurate
			// user-/topic- based propagation, we copy the uid and topics of the W to
			// all Rs.  Retweets are expected to have the same topics, although
			// people can modify/add new ones too.
			for (auto vi: by_youtube_video_ids) {
				bool first = true;
				vector<string>* first_topics = NULL;
				long first_uid = -1;
				for (auto op: vi.second) {
					if (first) {
						op->type = Tweet::Type::W;
						first_topics = &(op->topics);
						first_uid = op->uid;
						op->youtube_video_uploader = first_uid;
						first = false;
					} else {
						op->type = Tweet::Type::R;
						op->topics = *(first_topics);
						op->youtube_video_uploader = first_uid;
					}
				}
			}
		}

		{
			Util::CpuTimer _("Filtering out lonely tweets ...\n", 2);
			set<Tweet*> lonely_tweets;
			for (auto vi: by_youtube_video_ids) {
				if (vi.second.size() == 1)
					lonely_tweets.insert(*vi.second.begin());
			}

			for (auto it = tweets_before.begin(); it != tweets_before.end(); ) {
				if (lonely_tweets.find(*it) == lonely_tweets.end())
					tweets_after.push_back(*it);
			}

			cout << boost::format("  size: before %d, after %d (%.2f%%)\n")
				% before
				% tweets_after.size() % (100.0 * tweets_after.size() / before);
		}
	}

	void _FilterOutRepeatedAccessFromSameUser(
			const list<Tweet*>& tweets_before
			, list<Tweet*>& tweets_after) {
		Util::CpuTimer _("Filtering out repeated accesses from the same user ...\n");

		struct UidVid {
			long uid;
			string vid;	// youtube video id

			UidVid(Tweet* e)
				: uid(e->uid), vid(e->youtube_video_id)
			{}

			bool operator< (const UidVid& r) const {
				if (uid < r.uid) return true;
				if (uid > r.uid) return false;
				return vid < r.vid;
			}
		};

		set<UidVid> uid_vid_pairs;
		for (auto it = tweets_before.begin(); it != tweets_before.end(); ) {
			UidVid k(*it);
			if (uid_vid_pairs.find(k) == uid_vid_pairs.end()) {
				tweets_after.push_back(*it);
				uid_vid_pairs.insert(k);
			}
		}

		cout << boost::format("  before %d, after %d (%.2f%%)\n")
			% tweets_before.size()
			% tweets_after.size() % (100.0 * tweets_after.size() / tweets_before.size());
	}

	void _WriteTweetsToFile(const list<Tweet*> tweets) {
		Util::CpuTimer _("Writing tweets to file ...\n");
		size_t e_size = tweets.size();
		const string fn = str(boost::format("%s/%s-tweets-%d")
				% Conf::dn_data_home % _cur_datetime % e_size);
		ofstream ofs(fn, ios::binary);
		if (! ofs.is_open())
			throw runtime_error(str(boost::format("unable to open file %1%") % fn));
		ofs.write((char*)&e_size, sizeof(size_t));
		for (auto o: tweets)
			o->Write(ofs);
		ofs.close();
		cout << "  Generated file " << fn << " " << boost::filesystem::file_size(fn) << "\n";
	}

	void _FilterOutUsersWoTweets(
			const list<Tweet*>& tweets
			, const list<long>& uids
			, list<long>& uids_with_tweet) {
		Util::CpuTimer _("Filtering out no-tweet users ...\n");

		set<long> users_with_tweets;
		for (auto t: tweets)
			users_with_tweets.insert(t->uid);

		for (auto it = uids.begin(); it != uids.end(); it ++) {
			if (users_with_tweets.find(*it) != users_with_tweets.end())
				uids_with_tweet.push_back(*it);
		}

		cout << boost::format("  before %d, after %d (%.2f%%)\n")
			% uids.size()
			% uids_with_tweet.size() % (100.0 * uids_with_tweet.size() / uids.size());
	}

	void _WriteUsersToFile(const list<long>& uids) {
		Util::CpuTimer _("Writing users to file ...\n");
		size_t s = uids.size();
		const string fn = str(boost::format("%s/%s-users-%d")
				% Conf::dn_data_home % _cur_datetime % s);
		ofstream ofs(fn, ios::binary);
		if (! ofs.is_open())
			throw runtime_error(str(boost::format("unable to open file %1%") % fn));

		ofs.write((char*)&s, sizeof(size_t));
		for (long uid: uids)
			ofs.write((char*)&uid, sizeof(uid));
		ofs.close();
		cout << "  Generated file " << fn << " size=" << boost::filesystem::file_size(fn) << "\n";
	}

	void Gen() {
		// uids ordered by crawled_at
		list<long> uids_all;
		_LoadUsersFromDB(uids_all);

		list<Tweet*> tweets_all;
		_LoadTweetsFromDB(tweets_all);

		// User percent from 5% to 100% with a 5% increment
		for (int users_percent = 5; users_percent <= 100; users_percent += 5) {
			list<long> uids;
			size_t s = uids_all.size() * users_percent / 100;
			size_t i = 0;
			for (const auto u: uids_all) {
				uids.push_back(u);
				if (i >= s)
					break;
				i ++;
			}

			list<Tweet*> tweets_with_user;
			_FilterOutTweetsWoUser(uids, tweets_all, tweets_with_user);

			// For now, only tweets_with_user is used for the experiment.
			_WriteTweetsToFile(tweets_with_user);

			//list<Tweet*> tweets_wo_lonely;
			//_SetUploaderTopicsAndFilterOutLonelyTweets(tweets_with_user, tweets_wo_lonely);

			//list<Tweet*> tweets_wo_lonely_wo_repeated_accesses_from_the_same_user;
			//_FilterOutRepeatedAccessFromSameUser(tweets_wo_lonely, tweets_wo_lonely_wo_repeated_accesses_from_the_same_user);

			// Not used for now.
			//list<long> uids_with_tweets;
			//_FilterOutUsersWoTweets(tweets_with_user, uids, uids_with_tweets);
			//_WriteUsersToFile(uids_with_tweets);
		}

		for (auto e: tweets_all)
			delete e;
	}

	void Cleanup() {
		delete _conn;
		delete _stmt;
	}
}
