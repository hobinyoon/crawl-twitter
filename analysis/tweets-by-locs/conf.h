#ifndef __CONF_H__
#define __CONF_H__

#include <string>

namespace Conf {
	extern const std::string fn_tweets;

	extern bool stat;
	extern bool plot;

	extern size_t partial_load;

	extern const std::string fn_result;
	extern const std::string fn_plot;

	void Init(int argc, char* argv[]);
	const std::string& Desc();
};

#endif
