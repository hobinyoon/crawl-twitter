#include <libgen.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <thread>
#include <boost/algorithm/string/join.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include "conf.h"
#include "util.h"

using namespace std;

namespace Conf {
	const string dn_data_home = str(boost::format("%s/work/acorn-data") % getenv("HOME"));
	const string fn_tweets = dn_data_home + "/tweets-100";
	//const string fn_tweets = dn_data_home + "/tweets-w-lonely";

	string _dn_output;
	string fn_output;

	string _desc;

	void _ParseArgs(int argc, char* argv[]) {
		namespace po = boost::program_options;

		po::options_description od("Allowed options");
		od.add_options()
			//("partial_load", po::value<size_t>()->default_value(0), "Load partial reviews. Unlimited when 0.")
			("help", "show help message")
			;

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(od).run(), vm);
		po::notify(vm);

		if (vm.count("help") > 0) {
			// well... this doesn't show boolean as string.
			cout << std::boolalpha;

			cout << od << "\n";
			exit(0);
		}

		//partial_load = vm["partial_load"].as<size_t>();

		stringstream ss;
		ss << "Conf:\n";
		ss << std::boolalpha;
		//ss << "  partial_load=" << partial_load << "\n";
		ss << "  fn_tweets=" << fn_tweets << "\n";
		ss << "  fn_output=" << fn_output << "\n";
		_desc = ss.str();
		cout << _desc;
	}

	void Init(int argc, char* argv[]) {
		_dn_output = str(boost::format("%s/.output") % boost::filesystem::path(__FILE__).parent_path().string());
		//cout << boost::format("_dn_output: %s\n") % _dn_output;
		boost::filesystem::create_directories(_dn_output);
		fn_output = str(boost::format("%s/datetime-num-tweets-cdf") % _dn_output);

		_ParseArgs(argc, argv);
	}

	const string& Desc() {
		return _desc;
	}
};
