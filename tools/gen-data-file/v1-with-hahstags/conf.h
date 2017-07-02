#pragma once

#include <string>

namespace Conf {
	extern const std::string db_host;
	extern const std::string db_name;
	extern const std::string db_user;
	extern const std::string db_pass;

	extern const std::string dn_data_home;

	void Init(int argc, char* argv[]);
};
