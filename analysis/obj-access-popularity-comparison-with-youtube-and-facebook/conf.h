#pragma once

#include <string>

namespace Conf {
	extern size_t partial_load;
	extern bool stat;
	extern bool plot;
	extern bool include_writes;

	extern const std::string fn_tweets;

	extern const std::string fn_num_reqs;
	extern const std::string fn_plot_num_reqs;

	extern const std::string fn_ccdf_num_reqs;
	extern const std::string fn_plot_ccdf_num_reqs;

	void Init(int argc, char* argv[]);
	const std::string& Desc();
};
