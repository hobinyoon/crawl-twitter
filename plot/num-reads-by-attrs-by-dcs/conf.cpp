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
	const string db_host = "localhost";
	const string db_name = "twitter2";
	const string db_user = "twitter";
	const string db_pass = "twitterpass";

	const string dn_prj_home = Util::HomeDir() + "/work/pgr";
	const string fn_dc_coord = dn_prj_home + "/conf/dc-coord";
	size_t max_dcs;

	bool gen_data_file;
	const string dn_data_home = "/mnt/mdc-data/pgr/twitter";
	const string fn_tweets = dn_data_home + "/tweets";
	const string fn_tweets_1rv_per_user = dn_data_home + "/tweets-1rvpu";

	int max_repeated_videos_per_user;

	size_t num_checkins_in_timeline = 20;

	void _ParseArgs(int argc, char* argv[]) {
		namespace po = boost::program_options;

		po::options_description od0("Allowed options");
		od0.add_options()
			("gen_data_file", "Build data file from database")
			("max_repeated_videos_per_user", po::value<int>()->default_value(1), "Maximum repeated videos per user. Unlimited when -1.")
			("max_dcs", po::value<size_t>()->default_value(0), "Maximum # of DCs. 0 for unlimited.")
			("load_partial_checkin_data", po::value<string>(), "1p, or 10p")
			//("concurrency", po::value<int>()->default_value(8), "popularity calculation concurrency (number of threads)")
			("help", "show help message")
			;

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(od0).run(), vm);
		po::notify(vm);

		if (vm.count("help") > 0) {
			cout << od0 << "\n";
			exit(0);
		}

		gen_data_file = (vm.count("gen_data_file") > 0);

		max_repeated_videos_per_user = vm["max_repeated_videos_per_user"].as<int>();
		if (max_repeated_videos_per_user != -1 && max_repeated_videos_per_user <= 0)
				throw runtime_error(str(boost::format("Invalid max_repeated_videos_per_user: %d") % max_repeated_videos_per_user));

		max_dcs = vm["max_dcs"].as<size_t>();

		// Note: this can be used when loading data from DB to save time when data
		// gets big
		if (vm.count("load_partial_checkin_data") == 0) {
		} else {
			string load_partial_checkin_data = vm["load_partial_checkin_data"].as<string>();
			if (load_partial_checkin_data == "1p") {
			} else if (load_partial_checkin_data == "10p") {
			}
		}

		cout << "Conf:\n";
		cout << std::boolalpha;
		cout << "  gen_data_file=" << gen_data_file << "\n";
		cout << "  max_repeated_videos_per_user=" << max_repeated_videos_per_user << "\n";
	}

	void Init(int argc, char* argv[]) {
		_ParseArgs(argc, argv);
	}
};
