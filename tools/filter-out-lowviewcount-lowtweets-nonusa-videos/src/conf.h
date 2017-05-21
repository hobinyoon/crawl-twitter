#pragma once

#include <string>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include "cons.h"
#include "util.h"

namespace Conf {
	extern boost::program_options::variables_map _vm;

	void Init(int argc, char* argv[]);

	template<class T>
	const T Get(const std::string& k) {
		auto it = _vm.find(k);
		if (it == _vm.end())
			THROW(boost::format("Key %s not found") % k);

		return it->second.as<T>();
	}

	const std::string GetFn(const std::string& k);
};
