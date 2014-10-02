#include <boost/format.hpp>
#include "conf.h"
#include "plot.h"
#include "util.h"

using namespace std;

namespace Plot {
	void Plot() {
		Util::CpuTimer("Plotting ...\n");

		string cmd = str(boost::format("gnuplot %s/_num-uniq-attrs-per-dc.gnuplot") % Util::SrcDir());

		setenv("FN_IN", Conf::fn_num_uniq_attrs_per_dc.c_str(), 1);
		setenv("FN_PLOT", Conf::fn_plot.c_str(), 1);

		string out = Util::ExecGetOutput(cmd);
		if (out.size() > 0)
			throw runtime_error(str(boost::format("error: %1%") % out));

		cout << "  created file " << Conf::fn_plot << "\n";
	}
};
