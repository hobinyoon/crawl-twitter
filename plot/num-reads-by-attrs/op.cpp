// sudo apt-get install -y libmysqlcppconn-dev
// http://dev.mysql.com/doc/connector-cpp/en/connector-cpp-examples-complete-example-1.html

#include <fstream>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include "conf.h"
#include "op.h"
#include "stat.h"
#include "util.h"

using namespace std;

namespace Ops {
	void _ReadStr(ifstream& ifs, string& str) {
		size_t s;
		ifs.read((char*)&s, sizeof(s));
		str.resize(s);
		ifs.read((char*)&str[0], s);
	}

	void _WriteStr(ofstream& ofs, const string& str) {
		size_t s = str.size();
		ofs.write((char*)&s, sizeof(s));
		ofs.write(str.c_str(), str.size());
	}

	Entry::Entry(long id_,
			long uid_,
			const string& created_at_,
			double geo_lati_,
			double geo_longi_,
			const string& youtube_video_id_,
			const string& topics_)
		: id(id_), uid(uid_), created_at_str(created_at_),
		geo_lati(geo_lati_), geo_longi(geo_longi_),
		youtube_video_id(youtube_video_id_)
	{
		created_at = boost::posix_time::time_from_string(created_at_str);

		static const auto sep = boost::is_any_of(" ");
		boost::split(topics, topics_, sep);
	}

	Entry::Entry(ifstream& ifs) {
		ifs.read((char*)&id, sizeof(id));
		ifs.read((char*)&uid, sizeof(uid));

		_ReadStr(ifs, created_at_str);
		created_at = boost::posix_time::time_from_string(created_at_str);

		ifs.read((char*)&geo_lati, sizeof(geo_lati));
		ifs.read((char*)&geo_longi, sizeof(geo_longi));
		_ReadStr(ifs, youtube_video_id);

		size_t topic_cnt;
		ifs.read((char*)&topic_cnt, sizeof(topic_cnt));
		for (size_t i = 0; i < topic_cnt; i ++) {
			string t;
			_ReadStr(ifs, t);
			topics.push_back(t);
		}
	}

	void Entry::Write(ofstream& ofs) {
		ofs.write((char*)&id, sizeof(id));
		ofs.write((char*)&uid, sizeof(uid));
		_WriteStr(ofs, created_at_str);
		ofs.write((char*)&geo_lati, sizeof(geo_lati));
		ofs.write((char*)&geo_longi, sizeof(geo_longi));
		_WriteStr(ofs, youtube_video_id);
		{
			size_t s = topics.size();
			ofs.write((char*)&s, sizeof(s));
		}
		for (auto& t: topics)
			_WriteStr(ofs, t);
	}

	static vector<Entry*> _entries;

	const std::vector<Entry*> Entries() {
		return _entries;
	}

	void _LoadTweetsFromDB() {
		Util::CpuTimer _("Loading tweets from DB ...\n", 2);
		unique_ptr<sql::Connection> conn(get_driver_instance()->
				connect("tcp://" + Conf::db_host + ":3306", Conf::db_user, Conf::db_pass));
		conn->setSchema(Conf::db_name);
		unique_ptr<sql::Statement> stmt(conn->createStatement());
		unique_ptr<sql::ResultSet> rs(stmt->executeQuery(
					"SELECT id, uid, created_at, geo_lati, geo_longi, youtube_video_id, hashtags from tweets "
					"ORDER BY created_at "));

		while (rs->next()) {
			//cout << boost::format("  %ld %10ld %s %10lf %11lf %s\n")
			//	% rs->getInt64("id")
			//	% rs->getInt64("uid")
			//	% rs->getString("created_at")
			//	% rs->getDouble("geo_lati")
			//	% rs->getDouble("geo_longi")
			//	% rs->getString("youtube_video_id");
			_entries.push_back(new Entry(rs->getInt64("id"),
					rs->getInt64("uid"),
					rs->getString("created_at"),
					rs->getDouble("geo_lati"),
					rs->getDouble("geo_longi"),
					rs->getString("youtube_video_id"),
					rs->getString("hashtags")));
		}
	}

	void _LoadTweetsFromFile() {
		Util::CpuTimer _("Loading tweets from file ...\n", 2);

		const string& fn = Conf::fn_tweets;
		ifstream ifs(fn.c_str(), ios::binary);
		if (! ifs.is_open())
			throw runtime_error(str(boost::format("unable to open file %1%") % fn));

		size_t e_size;
		ifs.read((char*)&e_size, sizeof(e_size));
		for (size_t i = 0; i < e_size; i ++)
			_entries.push_back(new Entry(ifs));
	}

	void GenDataFile() {
		Util::CpuTimer _("Gen data file from DB ...\n");
		_LoadTweetsFromDB();
		cout << "  _entries.size()=" << _entries.size() << "\n";

		const string& fn = Conf::fn_tweets;
		ofstream ofs(fn.c_str(), ios::binary);
		if (! ofs.is_open())
			throw runtime_error(str(boost::format("unable to open file %1%") % fn));

		size_t e_size = _entries.size();
		ofs.write((char*)&e_size, sizeof(size_t));

		for (auto o: _entries)
			o->Write(ofs);
		ofs.close();

		cout << "  Generated file " << fn << " size=" << boost::filesystem::file_size(fn) << "\n";
	}

	void _FilterOutRepeatedAccessFromSameUser() {
		if (Conf::max_repeated_videos_per_user == -1)
			return;

		Util::CpuTimer _(str(boost::format("Allow at most %d repeated access "
						"to the same video from the same user ...\n") % Conf::max_repeated_videos_per_user), 2);

		struct Key {
			long uid;
			string vid;	// youtube video id

			Key(Entry* e)
				: uid(e->uid), vid(e->youtube_video_id)
			{}

			bool operator< (const Key& r) const {
				if (uid < r.uid) return true;
				if (uid > r.uid) return false;
				return vid < r.vid;
			}
		};

		size_t before = _entries.size();

		map<Key, int> uid_vids_cnt;
		for (auto it = _entries.begin(); it != _entries.end(); ) {
			Key k(*it);
			int cnt = -1;
			if (uid_vids_cnt.find(k) == uid_vids_cnt.end()) {
				uid_vids_cnt[k] = 1;
				cnt = 1;
			} else {
				cnt = uid_vids_cnt[k] + 1;
				uid_vids_cnt[k] = cnt;
			}

			if (cnt > Conf::max_repeated_videos_per_user) {
				it = _entries.erase(it);
			} else {
				it ++;
			}
		}

		cout << boost::format("    _entries.size()= before %d, after %d (%.2f%%)\n")
			% before % _entries.size()
			% (100.0 * _entries.size() / before);
	}

	void Load() {
		Util::CpuTimer _("Loading tweets ...\n");

		if (Conf::load_from == "file") {
			_LoadTweetsFromFile();
		} else if (Conf::load_from == "db") {
			_LoadTweetsFromDB();
		}
		cout << "  _entries.size()=" << _entries.size() << "\n";

		_FilterOutRepeatedAccessFromSameUser();
	}

	void _NumReadsByVideos() {
		Util::CpuTimer _("Num reads by videos ...\n");

		// map<youtube_video_id, vector<Entry*> >
		map<string, vector<Entry*> > by_youtube_video_ids;
		for (auto e: _entries) {
			auto it = by_youtube_video_ids.find(e->youtube_video_id);
			if (it == by_youtube_video_ids.end()) {
				vector<Entry*> v;
				v.push_back(e);
				by_youtube_video_ids[e->youtube_video_id] = v;
			} else {
				it->second.push_back(e);
			}
		}

		vector<size_t> sizes;
		for (auto i: by_youtube_video_ids) {
			if (i.second.size() > 1)
				sizes.push_back(i.second.size() - 1);
		}

		double x[10];
		double y[10];
		Stat::CDF(sizes, x, y, true, 2);
	}

	void _NumReadsByUsers() {
		Util::CpuTimer _("Num reads by users ...\n");

		map<long, vector<Entry*> > by_uids;
		for (auto e: _entries) {
			auto it = by_uids.find(e->uid);
			if (it == by_uids.end()) {
				vector<Entry*> v;
				v.push_back(e);
				by_uids[e->uid] = v;
			} else {
				it->second.push_back(e);
			}
		}

		vector<size_t> sizes;
		for (auto i: by_uids) {
			if (i.second.size() > 1)
				sizes.push_back(i.second.size() - 1);
		}

		double x[10];
		double y[10];
		Stat::CDF(sizes, x, y, true, 2);
	}

	void _NumReadsByTopics() {
		Util::CpuTimer _("Num reads by topics ...\n");

		map<string, vector<Entry*> > by_topics;
		for (auto e: _entries) {
			for (auto& t: e->topics) {
				auto it = by_topics.find(t);
				if (it == by_topics.end()) {
					vector<Entry*> v;
					v.push_back(e);
					by_topics[t] = v;
				} else {
					it->second.push_back(e);
				}
			}
		}

		vector<size_t> sizes;
		for (auto i: by_topics) {
			if (i.second.size() > 1)
				sizes.push_back(i.second.size() - 1);
		}

		double x[10];
		double y[10];
		Stat::CDF(sizes, x, y, true, 2);
	}

	void NumReadsBy() {
		_NumReadsByVideos();
		_NumReadsByUsers();
		_NumReadsByTopics();
	}

	void FreeMem() {
		for (auto e: _entries)
			delete e;
	}

	boost::posix_time::ptime oldest_created_at;
}


std::ostream& operator<< (std::ostream& os, const Ops::Entry& op) {
	os << boost::format("%ld %10ld %s %s %10lf %11lf %s")
		% op.id
		% op.uid
		% op.created_at
		% op.type
		% op.geo_lati
		% op.geo_longi
		% op.youtube_video_id;
	return os;
}


std::ostream& operator<< (std::ostream& os, const Ops::Type& opt) {
	static const char* s[] = {"W", "R"};
	os << s[opt];
	return os;
}
