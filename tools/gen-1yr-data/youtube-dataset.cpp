#include <dirent.h>
#include <fstream>
#include <set>
#include <sys/types.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "conf.h"
#include "tweet.h"
#include "stat.h"
#include "util.h"
#include "youtube-dataset.h"

using namespace std;

namespace YoutubeDataset {
	vector<string> fns_src = {
		"150505-104600-tweets"
			, "150509-112837-tweets-942332"
			, "150602-160822-tweets-3070454"
			, "150616-090458-tweets-4298023"
			, "150812-143151-tweets-5667779"
	};


	// Not used
	void _GetSrcFileList() {
		const string& dn = Conf::dn_data;

		DIR* dirp = opendir(dn.c_str());
		struct dirent* dp;
		while ((dp = readdir(dirp)) != NULL) {
			cout << boost::format("%s\n") % dp->d_name;
		}
		closedir(dirp);
	}


	// Datetime format: 2011-01-12 13:19:25
	const string _dt_begin = "2013-11-01 00:00:00";
	const string _dt_end   = "2014-11-01 00:00:00";

	void _LoadTweets(const string& fn, vector<Tweet*>& tweets) {
		Util::CpuTimer _(str(boost::format("Loading tweets from file %s ...\n") % fn));

		ifstream ifs(fn, ios::binary);
		if (! ifs.is_open())
			throw runtime_error(str(boost::format("unable to open file %1%") % fn));

		size_t num_tweets_in_file;
		ifs.read((char*)&num_tweets_in_file, sizeof(num_tweets_in_file));
		for (size_t i = 0; i < num_tweets_in_file; i ++) {
			Tweet* t = new Tweet(ifs);

			// Filter by created_at
			if (t->created_at_str < _dt_begin) {
				delete t;
				continue;
			}
			if (_dt_end <= t->created_at_str) {
				delete t;
				break;
			}

			tweets.push_back(t);
		}
		cout << boost::format("  Loaded %d tweets out of %d (%.2f%%)\n")
			% tweets.size() % num_tweets_in_file
			% (100.0 * tweets.size() / num_tweets_in_file);
	}
	

	void _SetWR(vector<Tweet*>& tweets) {
		Util::CpuTimer _("Setting W and R requests ...\n");

		//set<string> vids;
		map<string, long> vid_uploader_id;

		// Set the first request as W, the others as R.
		for (const auto& t: tweets) {
			auto i = vid_uploader_id.find(t->youtube_video_id);
			if (i == vid_uploader_id.end()) {
				vid_uploader_id.emplace(t->youtube_video_id, t->uid);
				t->type = Type::W;
				t->youtube_video_uploader = t->uid;
			} else {
				t->type = Type::R;
				t->youtube_video_uploader = i->second;
			}
			//cout << boost::format("[%s]\n") % t->created_at_str;
		}

		// Verify Ws and Rs. Dump them in plain text.
		//for (const auto& t: tweets)
		//	cout << *t << "\n";

		// No need to deleted the orphaned writes at the end. They are examples of
		// how partial replication is useful.
	}
	
	
	void _Write(const string& fn, vector<Tweet*>& tweets) {
		Util::CpuTimer _(str(boost::format("Creating new file %s ...\n") % fn));

		ofstream ofs(fn, ios::binary);
		if (! ofs.is_open())
			throw runtime_error(str(boost::format("unable to open file %1%") % fn));

		size_t size = tweets.size();
		ofs.write((char*)&size, sizeof(size));
		//cout << boost::format("writing %d tweets\n") % size;

		int i = 0;
		for (const auto& t: tweets) {
			t->Write(ofs);
			i ++;
			//cout << boost::format("%s %d i=%d\n") % __FILE__ % __LINE__ % i;
		}

		ofs.close();
		cout << boost::format("  Created %s %d\n") % fn % boost::filesystem::file_size(fn);
	}
	

	void Gen1YrFiles() {
		for (const string& fn: fns_src) {
			string fn1 = str(boost::format("%s/%s") % Conf::dn_data % fn);
			cout << boost::format("%s\n") % fn1;

			vector<Tweet*> tweets;
			_LoadTweets(fn1, tweets);
			_SetWR(tweets);

			string fn_out = str(boost::format("%s-1yr") % fn1);
			_Write(fn_out, tweets);
		}
	}
};
