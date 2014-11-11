#ifndef __CONF_H__
#define __CONF_H__

#include <string>

namespace Conf {
	extern const std::string fn_dc_coord;
	extern size_t partial_load;
	extern int max_topics_per_dc;
	extern const std::string fn_tweets;

	void Init(int argc, char* argv[]);
};

#endif
