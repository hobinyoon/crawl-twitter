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
	bool gen_stat;
	bool gen_plot;
	size_t partial_load;

	const string dn_data_home = "/mnt/mdc-data/pgr/twitter";
	const string fn_tweets = dn_data_home + "/tweets";
	//const string fn_tweets = dn_data_home + "/tweets-1rvpu";

	const string fn_ibc_by_time_by_dcs = dn_data_home + "/stat/ibc-by-time-by-dcs";
	const string fn_fck_by_loc = dn_data_home + "/stat/fck-by-loc";
	const string fn_plot_by_time = dn_data_home + "/stat/ibc-by-time.pdf";
	const string fn_plot_by_loc = dn_data_home + "/stat/fck-by-loc.pdf";

	int max_topics_per_dc;

	void _ParseArgs(int argc, char* argv[]) {
		namespace po = boost::program_options;

		po::options_description od0("Allowed options");
		od0.add_options()
			("gen_stat", po::value<bool>()->default_value(true), "Generate stat.")
			("gen_plot", po::value<bool>()->default_value(true), "Generate plot.")
			("partial_load", po::value<size_t>()->default_value(0), "Load up to partial_load. 0 for unlimited.")
			("max_topics_per_dc", po::value<int>()->default_value(20), "Print up to max_topics_per_dc. 0 for unlimited.")
			("help", "show help message")
			;

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(od0).run(), vm);
		po::notify(vm);

		if (vm.count("help") > 0) {
			cout << od0 << "\n";
			exit(0);
		}

		gen_stat = vm["gen_stat"].as<bool>();
		gen_plot = vm["gen_plot"].as<bool>();
		partial_load = vm["partial_load"].as<size_t>();
		max_topics_per_dc = vm["max_topics_per_dc"].as<int>();

		cout << "Conf:\n";
		cout << std::boolalpha;
		cout << "  gen_stat=" << gen_stat << "\n";
		cout << "  gen_plot=" << gen_plot << "\n";
		cout << "  partial_load=" << partial_load << "\n";
		cout << "  max_topics_per_dc=" << max_topics_per_dc << "\n";
	}

	void Init(int argc, char* argv[]) {
		_ParseArgs(argc, argv);
	}
};
