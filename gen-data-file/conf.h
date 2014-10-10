#ifndef __CONF_H__
#define __CONF_H__

#include <string>

namespace Conf {
	extern const std::string db_host;
	extern const std::string db_name;
	extern const std::string db_user;
	extern const std::string db_pass;

	extern const std::string fn_users;
	extern const std::string fn_tweets;
	extern const std::string fn_tweets_1rv_per_user;

	void Init(int argc, char* argv[]);
};

#endif
