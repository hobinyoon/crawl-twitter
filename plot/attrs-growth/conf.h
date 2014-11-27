#ifndef __CONF_H__
#define __CONF_H__

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace Conf {
	extern const std::string db_host;
	extern const std::string db_name;
	extern const std::string db_user;
	extern const std::string db_pass;

	extern const std::string fn_dc_coord;

	extern bool cnt;
	extern bool plot;
	extern bool lonely;

	extern size_t max_dcs;

	extern const std::string fn_users;
	extern const std::string fn_tweets;
	extern const std::string fn_tweets_w_lonely;

	extern const std::string fn_num_uniq_attrs_growth;
	extern const std::string fn_plot_num_uniq_attrs_growth;

	extern const std::string fn_plot_num_uniq_users_growth_by_dcs;
	extern const std::string fn_plot_num_uniq_topics_growth_by_dcs;


	extern const std::string fn_num_uniq_attrs_by_dcs;
	extern const std::string fn_plot_num_uniq_attrs_by_dcs;

	extern const std::string fn_num_uniq_attrs_growth_by_dcs;
	extern const std::string fn_plot_num_uniq_attrs_growth_by_dcs;

	void Init(int argc, char* argv[]);
	const std::string& Desc();
};

#endif
