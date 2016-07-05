#ifndef __CONF_H__
#define __CONF_H__

#include <string>

namespace Conf {
	extern const std::string dn_data;

	extern const std::string dt_begin;
	extern const std::string dt_end;

	void Init(int argc, char* argv[]);
	const std::string& Desc();
};

#endif
