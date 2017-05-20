#include <iostream>

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>

#include "cons.h"
#include "conf.h"
#include "util.h"


using namespace std;

namespace Conf {
	namespace po = boost::program_options;
	po::variables_map _vm;

	void _ParseArgs(int argc, char* argv[]) {
		po::options_description od("Allowed options");
		od.add_options()
			("in_file", po::value<string>()->default_value("~/work/castnet-data/youtube/150505-104600-tweets-6mo-noads-109950"), "Input file name")
			("out_file", po::value<string>()->default_value(
				str(boost::format("%s/../.result/video-list") % boost::filesystem::path(__FILE__).parent_path().string())), "List of videos")
			("help", "show help message")
			;

		po::positional_options_description pod;
		pod.add("in_file", 1);

		po::store(po::command_line_parser(argc, argv).options(od).positional(pod).run(), _vm);
		po::notify(_vm);

		if (_vm.count("help") > 0) {
			// well... this doesn't show boolean as string.
			cout << std::boolalpha;
			cout << od << "\n";
			exit(0);
		}

		// Print all parameters
		Cons::P("Options:");
		for (const auto o: _vm) {
			Cons::P(boost::format("  %s=%s") % o.first % o.second.as<string>());
		}
	}

	void Init(int argc, char* argv[]) {
		_ParseArgs(argc, argv);
	}

	const string GetFn(const string& k) {
		// Use boost::regex. C++11 regex works from 4.9. Ubuntu 14.04 has g++ 4.8.4.
		//   http://stackoverflow.com/questions/8060025/is-this-c11-regex-error-me-or-the-compiler
		return boost::regex_replace(
				_vm[k].as<string>()
				, boost::regex("~")
				, Util::HomeDir());
	}
};
