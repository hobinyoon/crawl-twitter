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

	void GetTweets(map<string, vector<Tweet*> >& vids_tweets) {
		const string& fn = Conf::fn_tweets;
		Util::CpuTimer _(str(boost::format("Loading tweets from %s ...\n") % fn), 2);

		ifstream ifs(fn.c_str(), ios::binary);
		if (! ifs.is_open())
			throw runtime_error(str(boost::format("Unable to open file %1%") % fn));

		size_t num_tweets;
		ifs.read((char*)&num_tweets, sizeof(num_tweets));

		for (size_t i = 0; i < num_tweets; i ++) {
			Tweet* t = new Tweet(ifs);

			auto it = vids_tweets.find(t->youtube_video_id);
			if (it == vids_tweets.end()) {
				vector<Tweet*> tweets;
				tweets.push_back(t);
				vids_tweets[t->youtube_video_id] = tweets;
			} else {
				it->second.push_back(t);
			}
		}
		cout << "    vids_tweets.size()=" << vids_tweets.size() << ". " << num_tweets << " tweets.\n";
	}
};
