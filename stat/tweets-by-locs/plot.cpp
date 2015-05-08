#include <boost/filesystem.hpp>
#include "conf.h"
#include "plot.h"
#include "util.h"

using namespace std;

namespace Plot {
	void _PlotLocs() {
		const string& fn_out = Conf::fn_plot;
		Util::SetEnv("FN_IN", Conf::fn_result);
		Util::SetEnv("FN_OUT", fn_out);
		string cmd = "gnuplot " + Util::SrcDir() + "/_tweets-locs.gnuplot";
		string out = Util::RunSubprocess(cmd);
		cout << Util::Indent(out, 2);
		cout << "  created files " << fn_out << " size=" << boost::filesystem::file_size(fn_out) << "\n";
	}

	void Plot() {
		Util::CpuTimer _("Plotting ...\n");
		_PlotLocs();
	}
};
