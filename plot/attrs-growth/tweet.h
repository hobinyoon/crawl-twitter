#ifndef __TWEET_H__
#define __TWEET_H__

#include <fstream>
#include <string>
#include <vector>

class DC;

struct Tweet {
	enum Type {
		NA = 0,	// Not assigned yet
		W = 1,
		R = 2,
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

	DC* LocalDC();
};

std::ostream& operator<< (std::ostream& os, const Tweet& u);
std::ostream& operator<< (std::ostream& os, const Tweet::Type& t);

#endif
