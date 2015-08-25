#pragma once

#include <string>

namespace Conf {
	extern const std::string fn_dc_coord;
	extern std::string fn_input;
	extern std::string topic;
	extern std::string year_month_date_0;
	extern std::string year_month_date_1;
	extern std::string month_date_0;
	extern std::string month_date_1;
	extern double cluster_dist;
	extern std::string fn_output;

	extern const std::string fn_ibc_by_time_by_dcs;
	extern const std::string fn_fck_by_loc;
	extern const std::string fn_plot_by_time;
	extern const std::string fn_plot_by_loc;

	void Init(int argc, char* argv[]);
};
