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
	const string dn_data_home = Util::HomeDir() + "/work/acorn-data/";
	//const string fn_tweets = dn_data_home + "150505-104600-tweets";
	const string fn_tweets = dn_data_home + "150812-143151-tweets-5667779";
	// 150505-104600-tweets              70589382
	// 150505-104600-tweets-1yr          25943782
	// 150509-112837-tweets-942332      120307918
	// 150509-112837-tweets-942332-1yr   44169078
	// 150602-160822-tweets-3070454     395330792
	// 150602-160822-tweets-3070454-1yr 166473149
	// 150616-090458-tweets-4298023     552824442
	// 150616-090458-tweets-4298023-1yr 244847585
	// 150812-143151-tweets-5667779     727765402
	// 150812-143151-tweets-5667779-1yr 324638357

	// Do I want lonely tweets? Castnet doesn't need it. Acorn may need it to
	// show the cost reduction from partial replication: when a video is
	// uploaded, but not viewed by anyone.
	//const string fn_tweets = dn_data_home + "/tweets-w-lonely";

	bool stat;
	bool plot;

	size_t partial_load;

	const string dn_output = str(boost::format("%s/.output") % boost::filesystem::path(__FILE__).parent_path().string());
	const string fn_result = dn_output + "/youtube-op-locs";
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
		// Create the output directory if not exists
		boost::filesystem::create_directories(dn_output);

		_ParseArgs(argc, argv);
	}

	const string& Desc() {
		return _desc;
	}
};
