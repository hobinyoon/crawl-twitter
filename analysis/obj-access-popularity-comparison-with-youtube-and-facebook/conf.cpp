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
	size_t partial_load;

	const string dn_data_home = "/mnt/mdc-data/twitter";
	//const string fn_tweets = dn_data_home + "/tweets-1rvpu";
	//const string fn_tweets = dn_data_home + "/tweets";
	const string fn_tweets = dn_data_home + "/tweets-w-lonely";

	bool stat;
	bool plot;
	bool include_writes;

	const string fn_num_reqs = dn_data_home + "/stat/num-reqs-by-popularity";
	const string fn_plot_num_reqs = dn_data_home + "/plot/num-reqs-by-popularity.pdf";

	const string fn_ccdf_num_reqs = dn_data_home + "/stat/ccdf-num-reqs";
	const string fn_plot_ccdf_num_reqs = dn_data_home + "/plot/ccdf-num-reqs.pdf";

	string _desc;

	void _ParseArgs(int argc, char* argv[]) {
		namespace po = boost::program_options;

		po::options_description od0("Allowed options");
		od0.add_options()
			("partial_load", po::value<size_t>()->default_value(0), "Partial load in percent. 0 for all.")
			("stat", po::value<bool>()->default_value(true), "Generate stat.")
			("plot", po::value<bool>()->default_value(false), "Plot.")
			("include_writes", po::value<bool>()->default_value(true), "Include writes.")
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
		stat = vm["stat"].as<bool>();
		plot = vm["plot"].as<bool>();
		include_writes = vm["include_writes"].as<bool>();

		stringstream ss; 
		ss << "Conf:\n";
		ss << std::boolalpha;
		ss << "  partial_load=" << partial_load << "\n";
		ss << "  stat=" << stat << "\n";
		ss << "  plot=" << plot << "\n";
		ss << "  include_writes=" << include_writes << "\n";
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
