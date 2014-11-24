#ifndef __CONF_H__
#define __CONF_H__

#include <string>

namespace Conf {
	extern const std::string fn_dc_coord;
	extern size_t partial_load;
	extern bool gen_stat;
	extern bool gen_plot;
	extern int max_topics_per_dc;
	extern const std::string fn_tweets;
	extern const std::string fn_ibc_by_time_by_dcs;
	extern const std::string fn_fck_by_loc;
	extern const std::string fn_plot_by_time;
	extern const std::string fn_plot_by_loc;

	void Init(int argc, char* argv[]);
};

#endif
