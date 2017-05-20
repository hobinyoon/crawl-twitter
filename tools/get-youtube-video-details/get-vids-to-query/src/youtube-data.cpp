#include <fstream>
#include <mutex>
#include <thread>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>

#include "conf.h"
#include "cons.h"
#include "op-youtube.h"
#include "util.h"
#include "youtube-data.h"

using namespace std;

namespace YoutubeData {
	// youtube
	vector<Op*> _entries;

	const vector<Op*>& Entries() {
		return _entries;
	}

	void _LoadOps() {
		string fn = Conf::GetFn("in_file");
		string dn = boost::filesystem::path(fn).parent_path().string();
		string fn1 = boost::filesystem::path(fn).filename().string();

		// Unzip if a zipped file exist
		if (! boost::filesystem::exists(fn)) {
			string fn_7z = fn + ".7z";
			if (boost::filesystem::exists(fn_7z)) {
				string cmd = str(boost::format("cd %s && 7z e %s.7z") % dn % fn1);
				Util::RunSubprocess(cmd);
			} else
				THROW("Unexpected");
		}

		Cons::MT _(boost::format("Loading YouTube workload from file %s ...") % fn);

		ifstream ifs(fn, ios::binary);
		if (! ifs.is_open())
			THROW(boost::format("unable to open file %1%") % fn);

		size_t e_size;
		ifs.read((char*)&e_size, sizeof(e_size));

		long num_in_usa = 0;
		long num_outside_usa = 0;
		for (size_t i = 0; i < e_size; i ++) {
			Op* e = new OpYoutube(ifs);
			_entries.push_back(e);

			// Assume all operations are reads
			//if (e->type == Op::Type::W)
			//	_num_writes ++;
			//else if (e->type == Op::Type::R)
			//	_num_reads ++;

			if (e->in_usa == 'Y')
				num_in_usa ++;
			else
				num_outside_usa ++;

			if (i % 10000 == 0) {
				Cons::ClearLine();
				Cons::Pnnl(boost::format("%.2f%%") % (100.0 * i / e_size));
			}
		}
		Cons::ClearLine();
		Cons::P("100.00%");

		// 80% of YouTube's views are from outside of the U.S. [https://fortunelords.com/youtube-statistics]
		Cons::P(boost::format("loaded %d ops. in_usa=%d (%.2f%%). outside_usa=%d (%.2f%%)")
				% _entries.size()
				% num_in_usa
				% (100.0 * num_in_usa / _entries.size())
				% num_outside_usa
				% (100.0 * num_outside_usa / _entries.size())
				);
	}

	void _SetCreatedAtPt0(int start, int end) {
		//Cons::P(boost::format("%d %d") % start % end);
		for (int i = start; i < end; i ++) {
			Op* op = _entries[i];
			op->SetCreatedAtPt();
		}
	}

	void _SetCreatedAtPt() {
		Cons::MT _("Setting created_at_pt ...");
		bool parallel = true;

		if (parallel) {
			int num_cpus = std::thread::hardware_concurrency();
			int s = _entries.size();
			int num_items = int(ceil(float(s) / num_cpus));

			vector<thread*> threads;
			for (int i = 0; i < num_cpus; i ++) {
				int start = i * num_items;
				int end = std::min((i + 1) * num_items, s);

				thread* t = new thread(_SetCreatedAtPt0, start, end);
				threads.push_back(t);
			}

			for (auto t: threads) {
				t->join();
				delete t;
			}
		} else {
			for (Op* op: _entries)
				op->SetCreatedAtPt();
		}
	}

	// Filter out excessive Tweets with the same video from the same person in a short period of time.
	void _FilterOutNonUSA() {
		Cons::MT _("Filtering out non-USA tweets ...");

		vector<Op*> new_entries;
		int num_filtered_out = 0;

		for (Op* op: _entries) {
			// 'Y' or 'N'
			if (op->in_usa == 'N') {
				delete op;
				num_filtered_out ++;
			} else {
				new_entries.push_back(op);
			}
		}

		size_t num_before = _entries.size();
		_entries = new_entries;

		Cons::P(boost::format("filtered out %d (%.2f%%) tweets. Now %d tweets")
				% num_filtered_out
				% (100.0 * num_filtered_out / num_before)
				% new_entries.size());
	}

	void _FilterOutSingleView() {
		Cons::MT _("Filtering out single-view entries ...");

		map<string, int> vid_cnt;

		for (Op* op: _entries) {
			const string& vid = op->obj_id;

			auto it = vid_cnt.find(vid);
			if (it == vid_cnt.end()) {
				vid_cnt[vid] = 1;
			} else {
				it->second ++;
			}
		}

		// Delete the single-view entries
		vector<Op*> new_entries;
		int num_filtered_out = 0;
		for (Op* op: _entries) {
			const string& vid = op->obj_id;

			auto it = vid_cnt.find(vid);
			if (it != vid_cnt.end() && it->second == 1) {
				delete op;
				num_filtered_out ++;
			} else {
				new_entries.push_back(op);
			}
		}

		size_t num_before = _entries.size();
		_entries = new_entries;

		Cons::P(boost::format("filtered out %d (%.2f%%) tweets. Now %d tweets")
				% num_filtered_out
				% (100.0 * num_filtered_out / num_before)
				% new_entries.size());
	}

	void Load() {
		_LoadOps();
		//_SetCreatedAtPt();
	}

	void FilteroutNonUSA() {
		_FilterOutNonUSA();
		_FilterOutSingleView();
	}

	void StatNumAccByVideos() {
		map<string, int> vid_cnt;

		for (Op* op: _entries) {
			const string& vid = op->obj_id;
			auto it = vid_cnt.find(vid);
			if (it == vid_cnt.end()) {
				vid_cnt[vid] = 1;
			} else {
				it->second ++;
			}
		}

		struct VidCnt {
			string vid;
			int cnt;
			VidCnt(const string& vid_, int cnt_)
				: vid(vid_), cnt(cnt_)
			{}

			bool operator < (const VidCnt& r) const {
				if (cnt < r.cnt) return true;
				if (cnt > r.cnt) return false;
				return (vid < r.vid);
			}
		};

		vector<VidCnt> vidcnts;
		for (auto i: vid_cnt)
			vidcnts.push_back(VidCnt(i.first, i.second));
		sort(vidcnts.begin(), vidcnts.end());

		string fn = Conf::GetFn("out_file");
		string dn = boost::filesystem::path(fn).parent_path().string();
		boost::filesystem::create_directories(dn);

		ofstream ofs(fn);
		if (! ofs.is_open())
			THROW(boost::format("unable to open file %1%") % fn);

		string fmt = "%11s %4d";
		ofs << Util::BuildHeader(fmt, "vid cnt") << "\n";

		for (auto it = vidcnts.rbegin(); it != vidcnts.rend(); it ++) {
			// Videos with at least 10 accesses
			if (it->cnt < 10)
				break;
			ofs << boost::format(fmt + "\n") % it->vid % it->cnt;
		}
		ofs.close();
		Cons::P(boost::format("Created %s %d") % fn % boost::filesystem::file_size(fn));
	}

	void Save() {
		const string fn = str(boost::format("%s-noads-%d") % Conf::GetFn("in_file") % _entries.size());
		Cons::MT _(boost::format("Writing output file %s ...") % fn);

		ofstream ofs(fn, ios::binary);
		if (! ofs.is_open())
			THROW(boost::format("unable to open file %1%") % fn);

		size_t e_size = _entries.size();
		ofs.write((char*)&e_size, sizeof(e_size));

		for (Op* op0: _entries) {
			OpYoutube* op = static_cast<OpYoutube*>(op0);
			op->Write(ofs);
		}
		ofs.close();

		Cons::P(boost::format("Created %s %d") % fn % boost::filesystem::file_size(fn));
	}

	void FreeMem() {
		if (_entries.size() == 0)
			return;

		Cons::MT _("Freeing ops ...");
		for (auto e: _entries)
			delete e;
	}
}
