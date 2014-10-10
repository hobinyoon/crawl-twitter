#ifndef __DATA_FILE_READER_H__
#define __DATA_FILE_READER_H__

#include <map>
#include <vector>
#include "tweet.h"

namespace DataFileReader {
	void GetUsersByCrawledAt(std::vector<long>& uids_by_crawled_at);
	void GetTweets(std::map<long, std::vector<Tweet*> >& users_tweets);
};

#endif
