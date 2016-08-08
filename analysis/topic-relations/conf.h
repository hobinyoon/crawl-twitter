#pragma once

#include <string>
#include <yaml-cpp/yaml.h>

namespace Conf {
	extern size_t partial_load;
	extern int max_output_lines;
	extern const std::string fn_tweets;

	void Init(int argc, char* argv[]);

	YAML::Node Get(std::string k);
};
