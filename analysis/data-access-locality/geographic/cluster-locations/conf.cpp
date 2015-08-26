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
	const string dn_prj_home = Util::HomeDir() + "/work/coco";
	const string fn_dc_coord = dn_prj_home + "/conf/dc-coord";

	string fn_input;
	string topic;
	string year_month_date_0;
	string year_month_date_1;
	string month_date_0;
	string month_date_1;
	double cluster_dist;
	string fn_output;

	void _ParseArgs(int argc, char* argv[]) {
		namespace po = boost::program_options;

		po::options_description od_visible("Allowed options");
		od_visible.add_options()
			("fn_input", po::value<string>()->default_value(""), "Input file name.")
			("topic", po::value<string>()->default_value(""), "Topic.")
			("year_month_date_0", po::value<string>()->default_value(""), "Datetime begin in YYYY-MM-DD.")
			("year_month_date_1", po::value<string>()->default_value(""), "Datetime end in YYYY-MM-DD.")
			("month_date_0", po::value<string>()->default_value(""), "MM-DD begin")
			("month_date_1", po::value<string>()->default_value(""), "MM-DD end")
			("cluster_dist", po::value<double>()->default_value(5.0), "Dist among each points either in lati or longi.")
			("fn_output", po::value<string>()->default_value(""), "Output file name.")
			("help", "show help message")
			;

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(od_visible).run(), vm);
		po::notify(vm);

		if (vm.count("help") > 0) {
			cout << od_visible << "\n";
			exit(0);
		}

		fn_input = vm["fn_input"].as<string>();
		if (fn_input.size() == 0)
			THROW_RE(boost::format("Unexpected fn_input=[%s]") % fn_input);

		topic = vm["topic"].as<string>();
		if (topic.size() == 0)
			THROW_RE(boost::format("Unexpected topic=[%s]") % topic);

		year_month_date_0 = vm["year_month_date_0"].as<string>();
		year_month_date_1 = vm["year_month_date_1"].as<string>();
		month_date_0 = vm["month_date_0"].as<string>();
		month_date_1 = vm["month_date_1"].as<string>();
		if ((year_month_date_0.size() == 0 || year_month_date_1.size() == 0)
				&& (month_date_0.size() == 0 || month_date_1.size() == 0))
			THROW_RE(boost::format("Unexpected year_month_date_0=[%s] year_month_date_1=[%s] "
						"month_date_0=[%s] month_date_1=[%s]")
					% year_month_date_0 % year_month_date_1
					% month_date_0 % month_date_1);

		cluster_dist = vm["cluster_dist"].as<double>();

		fn_output = vm["fn_output"].as<string>();
		if (fn_output.size() == 0)
			THROW_RE(boost::format("Unexpected fn_output=[%s]") % fn_output);

		cout << "Conf:\n";
		cout << std::boolalpha;
		cout << "  fn_input=" << fn_input << "\n";
		cout << "  topic=" << topic << "\n";
		cout << "  year_month_date_0=" << year_month_date_0 << "\n";
		cout << "  year_month_date_1=" << year_month_date_1 << "\n";
		cout << "  month_date_0=" << month_date_0 << "\n";
		cout << "  month_date_1=" << month_date_1 << "\n";
		cout << "  cluster_dist=" << cluster_dist << "\n";
		cout << "  fn_output=" << fn_output << "\n";
	}

	void Init(int argc, char* argv[]) {
		_ParseArgs(argc, argv);
	}
};
