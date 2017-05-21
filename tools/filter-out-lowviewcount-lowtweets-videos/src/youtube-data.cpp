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

	void FilteroutLowviewcntLownumtweets() {
		Cons::MT _("Filtering out low-viwecnt-low-num-tweets tweets ...");

		const string& fn = Conf::GetFn("youtube_video_info_file");

		set<string> vids;

		ifstream ifs(fn);
		if (! ifs.is_open())
			THROW(boost::format("unable to open file %1%") % fn);

		// chZmtjD8rxw 1154       5140   hd    PT5M49S M.O.D. - I Got Bitches
		string line;
		auto sep = boost::is_any_of(" ");
		while (getline(ifs, line)) {
			if (line.size() == 0)
				continue;
			if (line[0] == '#')
				continue;
			vector<string> t;
			split(t, line, sep, boost::token_compress_on);
			if (t.size() < 6)
				THROW(boost::format("Unexpected: [%s]") % line);
			const string& id = t[0];

			int num_locs = atoi(t[1].c_str());
			if (num_locs < 5)
				continue;

			if (t[2] == "None")
				continue;
			long view_cnt = atol(t[2].c_str());
			if (view_cnt < 50000)
				continue;

			vids.insert(id);
		}
		Cons::P(boost::format("Found %d videos that have 5 or more Tweets and 50000 or more view counts") % vids.size());

		// Filter out low-tweet-low-viewcnt videos
		vector<Op*> new_entries;
		int num_filtered_out = 0;
		for (Op* op: _entries) {
			auto it = vids.find(op->obj_id);
			if (it == vids.end()) {
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

	void Save() {
		const string fn = str(boost::format("%s-nolowviewcnts-%d") % Conf::GetFn("in_file") % _entries.size());
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
