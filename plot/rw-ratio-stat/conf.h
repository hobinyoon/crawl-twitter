#ifndef __CONF_H__
#define __CONF_H__

#include <string>

namespace Conf {
	extern const std::string fn_users;
	extern const std::string fn_tweets;

	extern const std::string fn_rw_ratio_histo;
	extern const std::string fn_plot;

	extern bool gen_histo;
	extern bool plot;

	void Init(int argc, char* argv[]);
};

#endif
