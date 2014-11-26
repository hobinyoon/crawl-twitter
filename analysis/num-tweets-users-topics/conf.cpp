#include <iostream>
#include <string>
#include <boost/algorithm/string/join.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include "conf.h"
#include "util.h"

using namespace std;

namespace Conf {
	const string dn_prj_home = Util::HomeDir() + "/work/pgr";
	const string fn_dc_coord = dn_prj_home + "/conf/dc-coord";
	size_t partial_load;

	const string dn_data_home = "/mnt/mdc-data/pgr/twitter";
	//const string fn_tweets = dn_data_home + "/tweets-w-lonely";
	//const string fn_tweets = dn_data_home + "/tweets-1rvpu";
	const string fn_tweets = dn_data_home + "/tweets";

	const string fn_top_topics = dn_data_home + "/top-topics";

	void _ParseArgs(int argc, char* argv[]) {
		namespace po = boost::program_options;

		po::options_description od0("Allowed options");
		od0.add_options()
			("partial_load", po::value<size_t>()->default_value(0), "Load up to partial_load. 0 for unlimited.")
			("help", "show help message")
			;

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(od0).run(), vm);
		po::notify(vm);

		if (vm.count("help") > 0) {
			cout << od0 << "\n";
			exit(0);
		}

		partial_load = vm["partial_load"].as<size_t>();

		cout << "Conf:\n";
		cout << std::boolalpha;
		cout << "  partial_load=" << partial_load << "\n";
	}

	void Init(int argc, char* argv[]) {
		_ParseArgs(argc, argv);
	}
};
