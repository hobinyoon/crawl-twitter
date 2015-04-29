#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "cnt-num-reads.h"
#include "conf.h"
#include "data-file-reader.h"
#include "util.h"

using namespace std;

namespace CntNumReads {
	map<string, vector<Tweet*> > _vids_tweets;

	void Load() {
		Util::CpuTimer _("Loading ...\n");
		DataFileReader::GetTweets(_vids_tweets);
	}

	void GenNumReadsHisto() {
		Util::CpuTimer _("Gen num reads histogram ...\n");
		map<size_t, int> histo;

		for (auto i: _vids_tweets) {
			// we don't count writes. thus -1.
			size_t s = i.second.size() - 1;

			auto it = histo.find(s);
			if (it == histo.end()) {
				histo[s] = 1;
			} else {
				it->second ++;
			}
		}

		const string& fn = Conf::fn_rw_ratio_histo;
		{
			ofstream ofs(fn);
			if (! ofs.is_open())
				throw runtime_error(str(boost::format("unable to open file %1%") % fn));
			for (auto h: histo)
				ofs << h.first << " " << h.second << "\n";
		}
		cout << "  Created " << fn << " " << boost::filesystem::file_size(fn) << "\n";
	}

	void FreeMem() {
		Util::CpuTimer _("Freeing tweets ...\n");
		for (auto ut: _vids_tweets)
			for (auto t: ut.second)
				delete t;
	}
}
