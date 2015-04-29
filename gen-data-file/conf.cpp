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

	const string dn_prj_home = Util::HomeDir() + "/work/pgr";
	const string dn_data_home = "/mnt/data/twitter";
	const string fn_users = dn_data_home + "/users";

	const string fn_tweets_w_lonely = dn_data_home + "/tweets-w-lonely";
	const string fn_tweets = dn_data_home + "/tweets";
	const string fn_tweets_1rv_per_user = dn_data_home + "/tweets-1rvpu";

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
