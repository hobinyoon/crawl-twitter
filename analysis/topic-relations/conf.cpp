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
	YAML::Node _config;

	size_t partial_load;

	// TODO: is this for 1 year or 6 months?
	const string fn_tweets = Util::HomeDir() + "/work/acorn-data/150812-143151-tweets-5667779-1yr";

	int max_output_lines;

	void _ParseArgs(int argc, char* argv[]) {
		namespace po = boost::program_options;

		po::options_description od0("Allowed options");
		od0.add_options()
			("partial_load", po::value<size_t>()->default_value(0), "Load up to partial_load. 0 for unlimited.")
			("max_output_lines", po::value<int>()->default_value(1000), "Maximum number of output lines. 0 for unlimited.")
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
		max_output_lines = vm["max_output_lines"].as<int>();

		// TODO: move these to the yaml
		cout << "Conf:\n";
		cout << std::boolalpha;
		cout << "  partial_load=" << partial_load << "\n";
		cout << "  max_output_lines=" << max_output_lines << "\n";
	}

	void _LoadYaml() {
		string fn = str(boost::format("%s/config.yaml") % boost::filesystem::path(__FILE__).parent_path().string().c_str());
		_config = YAML::LoadFile(fn);
	}

	void Init(int argc, char* argv[]) {
		_ParseArgs(argc, argv);
		_LoadYaml();
	}

	YAML::Node Get(string k) {
		return _config[k];
	}
};
