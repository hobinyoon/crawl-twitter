#pragma once

#include "tweet.h"

namespace DB {
  void Init();
  void GetUniqUsersFromCrawledTweets(std::vector<long>& tweets);
  void Cleanup();
};
