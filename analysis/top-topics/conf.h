#pragma once

#include <string>

namespace Conf {
	extern const std::string fn_dc_coord;
	extern size_t partial_load;
	extern int max_output_lines;
	extern const std::string fn_tweets;

	extern const std::string fn_top_topics;

	void Init(int argc, char* argv[]);
};
