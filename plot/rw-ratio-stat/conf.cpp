#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include "conf.h"
#include "util.h"

using namespace std;

namespace Conf {
	const string dn_prj_home = Util::HomeDir() + "/work/pgr";

	const string dn_data_home = "/mnt/mdc-data/pgr/twitter";
	const string fn_users = dn_data_home + "/users";
	const string fn_tweets = dn_data_home + "/tweets";

	const string fn_rw_ratio_histo = dn_data_home + "/stat/rw-ratio-per-video";
	const string fn_plot = fn_rw_ratio_histo + ".pdf";

	bool gen_histo;
	bool plot;

	void _ParseArgs(int argc, char* argv[]) {
		namespace po = boost::program_options;

		po::options_description od0("Allowed options");
		od0.add_options()
			("gen_histo", po::value<bool>()->default_value(true), "generate W:R ratio histogram")
			("plot", po::value<bool>()->default_value(true), "plot chart")
			("help", "show help message")
			;

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(od0).run(), vm);
		po::notify(vm);

		if (vm.count("help") > 0) {
			cout << od0 << "\n";
			exit(0);
		}

		gen_histo = vm["gen_histo"].as<bool>();
		plot = vm["plot"].as<bool>();

		cout << "Conf:\n";
		cout << std::boolalpha;
		cout << "  gen_histo=" << gen_histo << "\n";
		cout << "  plot=" << plot << "\n";
	}

	void Init(int argc, char* argv[]) {
		_ParseArgs(argc, argv);
	}
};
