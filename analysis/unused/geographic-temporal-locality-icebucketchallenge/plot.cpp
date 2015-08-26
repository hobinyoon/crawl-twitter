#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include "conf.h"
#include "plot.h"
#include "util.h"

using namespace std;

namespace Plot {
	void _ByTime() {
		string cmd = str(boost::format("gnuplot %s/_ibc-by-time.gnuplot") % Util::SrcDir());
		const string& fn_plot = Conf::fn_plot_by_time;
		setenv("FN_IN", Conf::fn_ibc_by_time_by_dcs.c_str(), 1);
		setenv("FN_PLOT", fn_plot.c_str(), 1);
		string out = Util::ExecGetOutput(cmd);
		if (out.size() > 0)
			throw runtime_error(str(boost::format("error: %1%") % out));
		cout << "  created file " << fn_plot << " " << boost::filesystem::file_size(fn_plot) << "\n";
	}

	void _ByLoc() {
		string cmd = str(boost::format("gnuplot %s/_ibc-by-loc.gnuplot") % Util::SrcDir());
		const string& fn_plot = Conf::fn_plot_by_loc;
		setenv("FN_IN", Conf::fn_fck_by_loc.c_str(), 1);
		setenv("FN_PLOT", fn_plot.c_str(), 1);
		string out = Util::ExecGetOutput(cmd);
		if (out.size() > 0)
			throw runtime_error(str(boost::format("error: %1%") % out));
		cout << "  created file " << fn_plot << " " << boost::filesystem::file_size(fn_plot) << "\n";
	}

	void Plot() {
		Util::CpuTimer _("Plotting ...\n");

		_ByTime();
		_ByLoc();
	}
};
