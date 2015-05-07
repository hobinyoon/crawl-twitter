#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include "conf.h"
#include "util.h"

using namespace std;

namespace Conf {
	const string db_host = "localhost";
	const string db_name = "twitter3";
	const string db_user = "twitter";
	const string db_pass = "twitterpass";

	const string dn_data_home = "/mnt/data/twitter";

	void _ParseArgs(int argc, char* argv[]) {
		namespace po = boost::program_options;

		po::options_description od0("Allowed options");
		od0.add_options()
			("help", "show help message")
			;

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(od0).run(), vm);
		po::notify(vm);

		if (vm.count("help") > 0) {
			cout << od0 << "\n";
			exit(0);
		}

		cout << "Conf:\n";
		cout << std::boolalpha;
	}

	void Init(int argc, char* argv[]) {
		_ParseArgs(argc, argv);
	}
};
