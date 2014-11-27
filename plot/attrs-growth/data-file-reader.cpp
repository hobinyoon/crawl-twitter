#include <fstream>
#include <boost/format.hpp>
#include "conf.h"
#include "data-file-reader.h"
#include "tweet.h"
#include "util.h"

using namespace std;

namespace DataFileReader {
	void GetUsersByCrawledAt(vector<long>& uids_by_crawled_at) {
		const string& fn = Conf::fn_users;
		Util::CpuTimer _(str(boost::format("Loading users from %s ...\n") % fn), 2);

		ifstream ifs(fn.c_str(), ios::binary);
		if (! ifs.is_open())
			throw runtime_error(str(boost::format("Unable to open file %1%") % fn));

		size_t num_users;
		ifs.read((char*)&num_users, sizeof(num_users));

		for (size_t i = 0; i < num_users; i ++) {
			long uid;
			ifs.read((char*)&uid, sizeof(uid));
			uids_by_crawled_at.push_back(uid);
		}

		cout << "    uids_by_crawled_at.size()=" << uids_by_crawled_at.size() << "\n";
	}

	void GetTweets(map<long, vector<Tweet*> >& users_tweets) {
		string fn;
		if (Conf::lonely)
			fn = Conf::fn_tweets_w_lonely;
		else
			fn = Conf::fn_tweets;

		Util::CpuTimer _(str(boost::format("Loading tweets from %s ...\n") % fn), 2);

		ifstream ifs(fn.c_str(), ios::binary);
		if (! ifs.is_open())
			throw runtime_error(str(boost::format("Unable to open file %1%") % fn));

		size_t num_tweets;
		ifs.read((char*)&num_tweets, sizeof(num_tweets));

		for (size_t i = 0; i < num_tweets; i ++) {
			Tweet* t = new Tweet(ifs);

			auto it = users_tweets.find(t->uid);
			if (it == users_tweets.end()) {
				vector<Tweet*> users;
				users.push_back(t);
				users_tweets[t->uid] = users;
			} else {
				it->second.push_back(t);
			}
		}
		cout << "    users_tweets.size()=" << users_tweets.size() << ". " << num_tweets << " tweets.\n";
	}
};
