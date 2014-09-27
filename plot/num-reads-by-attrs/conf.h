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
	extern std::string load_from;
	extern int max_repeated_videos_per_user;
	extern const std::string fn_tweets;

	extern size_t num_checkins_in_timeline;

	void Init(int argc, char* argv[]);
};

#endif
