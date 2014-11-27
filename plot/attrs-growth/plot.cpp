#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include "conf.h"
#include "plot.h"
#include "util.h"

using namespace std;

namespace Plot {
	void _UniqAttrsGrowth() {
		string cmd = str(boost::format("gnuplot %s/_num-uniq-attrs-growth.gnuplot") % Util::SrcDir());
		const string& fn_plot = Conf::fn_plot_num_uniq_attrs_growth;
		Util::SetEnv("FN_IN", Conf::fn_num_uniq_attrs_growth);
		Util::SetEnv("FN_PLOT", fn_plot);
		string out = Util::ExecGetOutput(cmd);
		if (out.size() > 0)
			throw runtime_error(str(boost::format("error: %1%") % out));
		cout << "  created file " << fn_plot << " " << boost::filesystem::file_size(fn_plot) << "\n";
	}

	void _UniqUsersGrowthByDCs() {
		string cmd = str(boost::format("gnuplot %s/_num-uniq-users-growth-by-dcs.gnuplot") % Util::SrcDir());
		const string& fn_plot = Conf::fn_plot_num_uniq_users_growth_by_dcs;
		Util::SetEnv("FN_IN", Conf::fn_num_uniq_attrs_growth_by_dcs);
		Util::SetEnv("FN_PLOT", fn_plot);
		string out = Util::ExecGetOutput(cmd);
		if (out.size() > 0)
			throw runtime_error(str(boost::format("error: %1%") % out));
		cout << "  created file " << fn_plot << " " << boost::filesystem::file_size(fn_plot) << "\n";
	}

	void _UniqTopicsGrowthByDCs() {
		string cmd = str(boost::format("gnuplot %s/_num-uniq-topics-growth-by-dcs.gnuplot") % Util::SrcDir());
		const string& fn_plot = Conf::fn_plot_num_uniq_topics_growth_by_dcs;
		Util::SetEnv("FN_IN", Conf::fn_num_uniq_attrs_growth_by_dcs);
		Util::SetEnv("FN_PLOT", fn_plot);
		string out = Util::ExecGetOutput(cmd);
		if (out.size() > 0)
			throw runtime_error(str(boost::format("error: %1%") % out));
		cout << "  created file " << fn_plot << " " << boost::filesystem::file_size(fn_plot) << "\n";
	}

	void Plot() {
		Util::CpuTimer _("Plotting ...\n");

		_UniqAttrsGrowth();
		_UniqUsersGrowthByDCs();
		_UniqTopicsGrowthByDCs();
	}
};
