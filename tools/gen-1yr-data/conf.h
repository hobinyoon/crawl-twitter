#ifndef __CONF_H__
#define __CONF_H__

#include <string>

namespace Conf {
	extern const std::string dn_data;
	extern const std::string fn_tweets;

	extern std::string fn_output;

	void Init(int argc, char* argv[]);
	const std::string& Desc();
};

#endif
