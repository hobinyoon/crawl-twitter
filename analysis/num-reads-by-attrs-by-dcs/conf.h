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
	extern size_t max_dcs;

	extern bool gen_data_file;
	extern int max_repeated_videos_per_user;
	extern const std::string fn_tweets;
	extern const std::string fn_tweets_1rv_per_user;
	extern const std::string fn_num_reads_by_vids;
	extern const std::string fn_num_reads_by_uids;
	extern const std::string fn_num_reads_by_topics;
	extern const std::string fn_plot;

	extern size_t num_checkins_in_timeline;

	void Init(int argc, char* argv[]);
};

#endif
