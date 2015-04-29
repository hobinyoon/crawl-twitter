#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include "conf.h"
#include "plot.h"
#include "util.h"

using namespace std;

namespace Plot {
	void _WRRatio() {
		string cmd = str(boost::format("gnuplot %s/_rw-ratio.gnuplot") % Util::SrcDir());
		const string& fn_plot = Conf::fn_plot;
		setenv("FN_IN", Conf::fn_rw_ratio_histo.c_str(), 1);
		setenv("FN_PLOT", fn_plot.c_str(), 1);
		string out = Util::ExecGetOutput(cmd);
		if (out.size() > 0)
			throw runtime_error(str(boost::format("error: %1%") % out));
		cout << "  created file " << fn_plot << " " << boost::filesystem::file_size(fn_plot) << "\n";
	}

	void Plot() {
		Util::CpuTimer _("Plotting ...\n");

		_WRRatio();
	}
};
