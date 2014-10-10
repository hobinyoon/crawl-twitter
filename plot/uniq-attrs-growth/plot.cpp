#include <boost/format.hpp>
#include "conf.h"
#include "plot.h"
#include "util.h"

using namespace std;

namespace Plot {
	void _UniqAttrsGrowth() {
		string cmd = str(boost::format("gnuplot %s/_num-uniq-attrs-growth.gnuplot") % Util::SrcDir());
		const string& fn_plot = Conf::fn_plot_num_uniq_attrs_growth;
		setenv("FN_IN", Conf::fn_num_uniq_attrs_growth.c_str(), 1);
		setenv("FN_PLOT", fn_plot.c_str(), 1);
		string out = Util::ExecGetOutput(cmd);
		if (out.size() > 0)
			throw runtime_error(str(boost::format("error: %1%") % out));
		cout << "  created file " << fn_plot << "\n";
	}

	void _UniqAttrsGrowthByDCs() {
		string cmd = str(boost::format("gnuplot %s/_num-uniq-attrs-growth-by-dcs.gnuplot") % Util::SrcDir());
		const string& fn_plot = Conf::fn_plot_num_uniq_attrs_growth_by_dcs;
		setenv("FN_IN", Conf::fn_num_uniq_attrs_growth_by_dcs.c_str(), 1);
		setenv("FN_PLOT", fn_plot.c_str(), 1);
		string out = Util::ExecGetOutput(cmd);
		if (out.size() > 0)
			throw runtime_error(str(boost::format("error: %1%") % out));
		cout << "  created file " << fn_plot << "\n";
	}

	void Plot() {
		Util::CpuTimer _("Plotting ...\n");

		_UniqAttrsGrowth();
		_UniqAttrsGrowthByDCs();
	}
};
