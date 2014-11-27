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

	bool cnt;
	bool plot;
	bool lonely;

	size_t max_dcs;

	const string dn_data_home = "/mnt/mdc-data/pgr/twitter";
	const string fn_users = dn_data_home + "/users";
	const string fn_tweets = dn_data_home + "/tweets";
	const string fn_tweets_w_lonely = dn_data_home + "/tweets-w-lonely";

	const string fn_num_uniq_attrs_growth = dn_data_home + "/stat/num-uniq-attrs-growth";
	const string fn_plot_num_uniq_attrs_growth = fn_num_uniq_attrs_growth + ".pdf";

	const string fn_plot_num_uniq_users_growth_by_dcs = dn_data_home + "/stat/num-uniq-users-growth-by-dcs.pdf";
	const string fn_plot_num_uniq_topics_growth_by_dcs = dn_data_home + "/stat/num-uniq-topics-growth-by-dcs.pdf";



	// TODO: clean up

	const string fn_num_uniq_attrs_by_dcs = dn_data_home + "/stat/num-uniq-attrs-by-dcs";
	const string fn_plot_num_uniq_attrs_by_dcs = fn_num_uniq_attrs_by_dcs + ".pdf";

	const string fn_num_uniq_attrs_growth_by_dcs = dn_data_home + "/stat/num-uniq-attrs-growth-by-dcs";
	const string fn_plot_num_uniq_attrs_growth_by_dcs = fn_num_uniq_attrs_growth_by_dcs + ".pdf";

	string _conf_desc;

	void _ParseArgs(int argc, char* argv[]) {
		namespace po = boost::program_options;

		po::options_description od0("Allowed options");
		od0.add_options()
			("cnt", po::value<bool>()->default_value(true), "Count attributes.")
			("plot", po::value<bool>()->default_value(true), "Plot charts.")
			("lonely", po::value<bool>()->default_value(false), "Include lonely tweets.")
			("max_dcs", po::value<size_t>()->default_value(0), "Maximum # of DCs. 0 for unlimited.")
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

		cnt = vm["cnt"].as<bool>();
		plot = vm["plot"].as<bool>();
		lonely = vm["lonely"].as<bool>();
		max_dcs = vm["max_dcs"].as<size_t>();

		stringstream ss;
		ss << "Conf:\n";
		ss << std::boolalpha;
		ss << "  cnt=" << cnt << "\n";
		ss << "  plot=" << plot << "\n";
		ss << "  lonely=" << lonely << "\n";
		//ss << "  max_repeated_videos_per_user=" << max_repeated_videos_per_user << "\n";

		_conf_desc = ss.str();
		cout << _conf_desc;
	}

	void Init(int argc, char* argv[]) {
		_ParseArgs(argc, argv);
	}

	const string& Desc() {
		return _conf_desc;
	}
};
