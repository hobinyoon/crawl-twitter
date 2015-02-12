#include <boost/filesystem.hpp>
#include "conf.h"
#include "plot.h"
#include "util.h"

using namespace std;

namespace Plot {
	void _NumReqsByPopRank() {
		const string& fn_in = Conf::fn_num_reqs;
		const string& fn_out = Conf::fn_plot_num_reqs;
		Util::SetEnv("FN_IN", fn_in);
		Util::SetEnv("FN_OUT", fn_out);
		string cmd = "gnuplot " + Util::SrcDir() + "/_num-reqs-by-pop-rank.gnuplot";
		string out = Util::RunSubprocess(cmd);
		if (out.size() > 0)
			cout << Util::Indent(out, 2);
		cout << "  created " << fn_out << " " << boost::filesystem::file_size(fn_out) << "\n";
	}

	void _CCDFNumReqs() {
		const string& fn_in = Conf::fn_ccdf_num_reqs;
		const string& fn_out = Conf::fn_plot_ccdf_num_reqs;
		Util::SetEnv("FN_IN", fn_in);
		Util::SetEnv("FN_OUT", fn_out);
		string cmd = "gnuplot " + Util::SrcDir() + "/_ccdf-num-reqs.gnuplot";
		string out = Util::RunSubprocess(cmd);
		if (out.size() > 0)
			cout << Util::Indent(out, 2);
		cout << "  created " << fn_out << " " << boost::filesystem::file_size(fn_out) << "\n";
	}

	void Plot() {
		Util::CpuTimer _("Plotting ...\n");
		_NumReqsByPopRank();
		_CCDFNumReqs();
	}
};
