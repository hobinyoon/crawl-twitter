#pragma once

#include <vector>

#include "tweet.h"

namespace DB {
  void Init();
  void GetAllTweets(std::vector<Tweet*>& tweets);
  void Cleanup();
};
