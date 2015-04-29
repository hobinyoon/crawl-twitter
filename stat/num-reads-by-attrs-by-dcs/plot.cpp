#include <boost/format.hpp>
#include "conf.h"
#include "plot.h"
#include "util.h"

using namespace std;

namespace Plot {
	void Plot() {
		Util::CpuTimer("Plotting ...\n");

		string cmd = str(boost::format("gnuplot %s/_num-reads-by-attrs-by-dcs.gnuplot") % Util::SrcDir());

		setenv("FN_BY_VIDS", Conf::fn_num_reads_by_vids.c_str(), 1);
		setenv("FN_BY_UIDS", Conf::fn_num_reads_by_uids.c_str(), 1);
		setenv("FN_BY_TOPICS", Conf::fn_num_reads_by_topics.c_str(), 1);
		setenv("FN_PLOT", Conf::fn_plot.c_str(), 1);

		string out = Util::ExecGetOutput(cmd);
		if (out.size() > 0)
			throw runtime_error(str(boost::format("error: %1%") % out));

		cout << "  created file " << Conf::fn_plot << "\n";
	}
};
