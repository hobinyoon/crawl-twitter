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
	const string dn_data_home = "/mnt/mdc-data/pgr/twitter";
	//const string fn_tweets = dn_data_home + "/tweets";
	const string fn_tweets = dn_data_home + "/tweets-w-lonely";

	bool stat;
	bool plot;

	size_t partial_load;

	const string fn_result = dn_data_home + "/stat/youtube-op-locs";
	const string fn_plot = fn_result + ".pdf";

	string _desc;

	void _ParseArgs(int argc, char* argv[]) {
		namespace po = boost::program_options;

		po::options_description od("Allowed options");
		od.add_options()
			("stat", po::value<bool>()->default_value(true), "Generate stat")
			("plot", po::value<bool>()->default_value(true), "Generate plot")
			("partial_load", po::value<size_t>()->default_value(0), "Load partial reviews. Unlimited when 0.")
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

		stat = vm["stat"].as<bool>();
		plot = vm["plot"].as<bool>();
		partial_load = vm["partial_load"].as<size_t>();

		stringstream ss;
		ss << "Conf:\n";
		ss << std::boolalpha;
		ss << "  stat=" << stat << "\n";
		ss << "  plot=" << plot << "\n";
		ss << "  partial_load=" << partial_load << "\n";
		ss << "  fn_tweets=" << fn_tweets << "\n";
		ss << "  fn_result=" << fn_result << "\n";
		ss << "  fn_plot=" << fn_plot << "\n";
		_desc = ss.str();
		cout << _desc;
	}

	void Init(int argc, char* argv[]) {
		_ParseArgs(argc, argv);
	}

	const string& Desc() {
		return _desc;
	}
};
