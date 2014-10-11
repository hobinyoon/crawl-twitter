#ifndef __TWEET_H__
#define __TWEET_H__

#include <string>
#include <vector>

struct Tweet {
	enum Type {
		W,
		R,
	};

	long id;
	long uid;
	std::string created_at;
	double geo_lati;
	double geo_longi;
	std::string youtube_video_id;
	long youtube_video_uploader;
	std::vector<std::string> topics;
	Type type;

	Tweet(long id_,
			long uid_,
			const std::string& created_at_,
			double geo_lati_,
			double geo_longi_,
			const std::string& youtube_video_id_,
			const std::string& topics_);

	Tweet(std::ifstream& ifs);

	void Write(std::ofstream& ofs);
};


std::ostream& operator<< (std::ostream& os, const Tweet& op);
std::ostream& operator<< (std::ostream& os, const Tweet::Type& opt);

#endif
