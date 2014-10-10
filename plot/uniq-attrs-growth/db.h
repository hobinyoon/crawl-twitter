#ifndef __DB_H__
#define __DB_H__

#include <map>
#include <vector>
#include "tweet.h"

namespace DB {
	void Init();
	void GetUsersByCrawledAt(std::vector<long>& uids_by_crawled_at);
	void GetTweets(std::map<long, std::vector<Tweet*> >& users_tweets);
	void Cleanup();
};

#endif
