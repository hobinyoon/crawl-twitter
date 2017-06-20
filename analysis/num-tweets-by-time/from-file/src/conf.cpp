#include <iostream>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/regex.hpp>

#include "conf.h"
#include "cons.h"
#include "util.h"


using namespace std;

namespace Conf {
	YAML::Node _yaml_root;

	void _LoadYaml() {
		string fn = str(boost::format("%s/../config.yaml") % boost::filesystem::path(__FILE__).parent_path().string());
		_yaml_root = YAML::LoadFile("config.yaml");
	}

	namespace po = boost::program_options;

	template<class T>
		void __EditYaml(const string& key, po::variables_map& vm) {
			if (vm.count(key) != 1)
				return;
			T v = vm[key].as<T>();
			static const auto sep = boost::is_any_of(".");
			vector<string> tokens;
			boost::split(tokens, key, sep, boost::token_compress_on);
			// Had to use a pointer to traverse the tree. Otherwise, the tree gets messed up.
			YAML::Node* n = &_yaml_root;
			for (string t: tokens) {
				YAML::Node n1 = (*n)[t];
				n = &n1;
			}
			*n = v;
			//Cons::P(Desc());
		}

	void _ParseArgs(int argc, char* argv[]) {
		po::options_description od("Allowed options");
		od.add_options()
			//("time_interval_granularity",
			// po::value<double>()->default_value(Conf::GetStr("time_interval_granularity")))
			("out_dir",
			 po::value<string>()->default_value(Conf::GetStr("out_dir")))
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

		__EditYaml<string>("out_dir", vm);

		// Print all parameters
		Cons::P("Options:");

		YAML::Node n = _yaml_root;
		for (YAML::const_iterator it = n.begin(); it != n.end(); ++ it) {
			string k = it->first.as<string>();
			if (it->second.IsScalar()) {
				Cons::P(boost::format("  %s: %s") % k % it->second.as<string>());
				// it->second.Type()
			} else {
				THROW("Implement");
			}
		}
		//for (const auto o: vm) {
		//	Cons::P(boost::format("  %s=%s") % o.first % o.second.as<string>());
		//}
	}

	void Init(int argc, char* argv[]) {
		// Might be useful for cache placement planning tool.
		//_exp_id = Util::CurDateTime();
		_LoadYaml();
		_ParseArgs(argc, argv);
	}

	YAML::Node Get(const std::string& k) {
		return _yaml_root[k];
	}

	string GetStr(const std::string& k) {
		return _yaml_root[k].as<string>();
	}

	string GetFn(const std::string& k) {
		return boost::regex_replace(GetStr(k), boost::regex("~"), Util::HomeDir());
	}

	const string Desc() {
		YAML::Emitter emitter;
		emitter << _yaml_root;
		if (! emitter.good())
			THROW("Unexpected");
		return emitter.c_str();
	}

	const string OutDir() {
		return str(boost::format("%s/../%s")
				% boost::filesystem::path(__FILE__).parent_path().string()
				% GetStr("out_dir"));
	}
};
