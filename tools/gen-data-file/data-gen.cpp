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

	// all tweets
	list<Tweet*> _tweets0;

	// after filtering out lonely tweets
	list<Tweet*> _tweets1;


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

	// uids by crawled_at
	list<long> _uids;

	void _LoadUsersFromDB() {
		Util::CpuTimer _("Loading users from DB ...\n");
		unique_ptr<sql::ResultSet> rs(_stmt->executeQuery(
					"SELECT id FROM users WHERE status='C' ORDER BY crawled_at"));
		int i = 0;
		while (rs->next()) {
			_uids.push_back(rs->getInt64("id"));
			i ++;
			if (i % 1000 == 0) {
				Util::ClearLine();
				cout << "  " << i << flush;
			}
		}
		Util::ClearLine();
		cout << "  _uids.size()=" << _uids.size() << "\n";
	}

	void _LoadTweetsFromDB() {
		Util::CpuTimer _("Loading tweets from DB ...\n");
		unique_ptr<sql::ResultSet> rs(_stmt->executeQuery(
					"SELECT id, uid, created_at, geo_lati, geo_longi, youtube_video_id, hashtags FROM tweets "
					// Useful during dev
					//"WHERE id < 380000000000000000 "
					"ORDER BY created_at "));

		int cnt = 0;
		while (rs->next()) {
			_tweets0.push_back(new Tweet(rs->getInt64("id"),
						rs->getInt64("uid"),
						rs->getString("created_at"),
						rs->getDouble("geo_lati"),
						rs->getDouble("geo_longi"),
						rs->getString("youtube_video_id"),
						rs->getString("hashtags")));
			cnt ++;
			if (cnt % 1000 == 0) {
				Util::ClearLine();
				cout << "  " << cnt << flush;
			}
		}
		Util::ClearLine();

		cout << "  _tweets0.size()=" << _tweets0.size() << "\n";

		//for (auto t: _tweets0)
		//	cout << "  " << *t << "\n";
	}

	// This filters out half-crawled tweets from users.
	void _FilterOutTweetsWoUser() {
		Util::CpuTimer _("Filtering out tweets without user info (should be rare) ...\n");

		size_t before = _tweets0.size();

		set<long> uid_set;
		for (auto uid: _uids)
			uid_set.insert(uid);

		for (auto it = _tweets0.begin(); it != _tweets0.end(); ) {
			if (uid_set.count((*it)->uid) == 1)
				it ++;
			else
				it = _tweets0.erase(it);
		}

		cout << boost::format("  _tweets0.size()= before %d, after %d (%.2f%%)\n")
			% before % _tweets0.size()
			% (100.0 * _tweets0.size() / before);
	}

	// filter out tweets that are the only ones with the video_id
	void _FilterOutLonelyTweetsSetUploaderTopics() {
		Util::CpuTimer _("Filtering out lonely tweets and setting uploader and topics ...\n");

		size_t before = _tweets0.size();

		map<string, vector<Tweet*> > by_youtube_video_ids;
		for (auto e: _tweets0) {
			auto it = by_youtube_video_ids.find(e->youtube_video_id);
			if (it == by_youtube_video_ids.end()) {
				vector<Tweet*> v;
				v.push_back(e);
				by_youtube_video_ids[e->youtube_video_id] = v;
			} else {
				it->second.push_back(e);
			}
		}

		_tweets1 = _tweets0;
		{
			Util::CpuTimer _("Filtering out lonely tweets ...\n", 2);
			set<Tweet*> lonely_tweets;
			for (auto vi: by_youtube_video_ids) {
				if (vi.second.size() == 1)
					lonely_tweets.insert(*vi.second.begin());
			}

			for (auto it = _tweets1.begin(); it != _tweets1.end(); ) {
				if (lonely_tweets.find(*it) != lonely_tweets.end())
					it = _tweets1.erase(it);
				else
					it ++;
			}

			cout << boost::format("  size: before %d, after %d (%.2f%%)\n")
				% before % _tweets1.size()
				% (100.0 * _tweets1.size() / before);
		}

		{
			Util::CpuTimer _("Setting uploader and topics ...\n", 2);
			// The first one is W and all the others are Rs. For more accurate
			// user-/topic- based propagation, we copy the uid and topics of the W to
			// all Rs.  Retweets are expected to have the same topics, although people
			// can modify/add new ones too.
			for (auto vi: by_youtube_video_ids) {
				if (vi.second.size() == 1)
					continue;

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
	}

	void _FilterOutRepeatedAccessFromSameUser() {
		Util::CpuTimer _("Filtering out repeated accesses from the same user ...\n", 2);

		struct Key {
			long uid;
			string vid;	// youtube video id

			Key(Tweet* e)
				: uid(e->uid), vid(e->youtube_video_id)
			{}

			bool operator< (const Key& r) const {
				if (uid < r.uid) return true;
				if (uid > r.uid) return false;
				return vid < r.vid;
			}
		};

		size_t before = _tweets1.size();

		map<Key, int> uid_vids_cnt;
		for (auto it = _tweets1.begin(); it != _tweets1.end(); ) {
			Key k(*it);
			int cnt = -1;
			if (uid_vids_cnt.find(k) == uid_vids_cnt.end()) {
				uid_vids_cnt[k] = 1;
				cnt = 1;
			} else {
				cnt = uid_vids_cnt[k] + 1;
				uid_vids_cnt[k] = cnt;
			}

			if (cnt > 1) {
				it = _tweets1.erase(it);
			} else {
				it ++;
			}
		}

		cout << boost::format("    _tweets1.size()= before %d, after %d (%.2f%%)\n")
			% before % _tweets1.size()
			% (100.0 * _tweets1.size() / before);
	}

	void _WriteTweetsToFile() {
		Util::CpuTimer _("Writing tweets to file ...\n");
		{
			size_t e_size = _tweets0.size();
			const string fn = str(boost::format("%s/%s-tweets-w-lonely-%d")
					% Conf::dn_data_home % _cur_datetime % e_size);
			ofstream ofs(fn, ios::binary);
			if (! ofs.is_open())
				throw runtime_error(str(boost::format("unable to open file %1%") % fn));
			ofs.write((char*)&e_size, sizeof(size_t));
			for (auto o: _tweets0)
				o->Write(ofs);
			ofs.close();
			cout << "  Generated file " << fn << " size=" << boost::filesystem::file_size(fn) << "\n";
		}
		{
			size_t e_size = _tweets1.size();
			const string fn = str(boost::format("%s/%s-tweets-%d")
					% Conf::dn_data_home % _cur_datetime % e_size);
			ofstream ofs(fn, ios::binary);
			if (! ofs.is_open())
				throw runtime_error(str(boost::format("unable to open file %1%") % fn));
			ofs.write((char*)&e_size, sizeof(size_t));
			for (auto o: _tweets1)
				o->Write(ofs);
			ofs.close();
			cout << "  Generated file " << fn << " size=" << boost::filesystem::file_size(fn) << "\n";
		}

		{
			_FilterOutRepeatedAccessFromSameUser();

			size_t e_size = _tweets1.size();
			const string fn = str(boost::format("%s/%s-tweets-1rvpu-%d")
					% Conf::dn_data_home % _cur_datetime % e_size);
			ofstream ofs(fn, ios::binary);
			if (! ofs.is_open())
				throw runtime_error(str(boost::format("unable to open file %1%") % fn));
			ofs.write((char*)&e_size, sizeof(size_t));
			for (auto o: _tweets1)
				o->Write(ofs);
			ofs.close();
			cout << "  Generated file " << fn << " size=" << boost::filesystem::file_size(fn) << "\n";
		}
	}

	void _FilterOutNoTweetUsers() {
		Util::CpuTimer _("Filtering out no-tweet users ...\n");

		size_t before = _uids.size();

		set<long> users_with_tweets;
		for (auto t: _tweets0)
			users_with_tweets.insert(t->uid);

		for (auto it = _uids.begin(); it != _uids.end(); ) {
			auto it2 = users_with_tweets.find(*it);
			if (it2 == users_with_tweets.end()) {
				it = _uids.erase(it);
			} else {
				it ++;
			}
		}

		cout << boost::format("  _uids.size()= before %d, after %d (%.2f%%)\n")
			% before % _uids.size()
			% (100.0 * _uids.size() / before);
	}

	void _WriteUsersToFile() {
		Util::CpuTimer _("Writing users to file ...\n");
		size_t s = _uids.size();
		const string fn = str(boost::format("%s/%s-users-%d")
				% Conf::dn_data_home % _cur_datetime % s);
		ofstream ofs(fn, ios::binary);
		if (! ofs.is_open())
			throw runtime_error(str(boost::format("unable to open file %1%") % fn));

		ofs.write((char*)&s, sizeof(size_t));
		for (long uid: _uids)
			ofs.write((char*)&uid, sizeof(uid));
		ofs.close();
		cout << "  Generated file " << fn << " size=" << boost::filesystem::file_size(fn) << "\n";
	}

	void Gen() {
		_LoadUsersFromDB();
		_LoadTweetsFromDB();

		_FilterOutTweetsWoUser();

		_FilterOutLonelyTweetsSetUploaderTopics();
		_WriteTweetsToFile();

		_FilterOutNoTweetUsers();
		_WriteUsersToFile();
	}

	void Cleanup() {
		for (auto e: _tweets0)
			delete e;

		delete _conn;
		delete _stmt;
	}
}
