#pragma once

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace Ops {
	enum Type {
		NA = 0,	// not assigned
		W = 1,
		R = 2,
	};

	struct Entry {
		long id;
		long uid;
		boost::posix_time::ptime created_at;
		std::string created_at_str;
		double geo_lati;
		double geo_longi;
		std::string youtube_video_id;
		long youtube_video_uploader;
		std::vector<std::string> topics;
		Type type;

		Entry(long id_,
				long uid_,
				const std::string& created_at_,
				double geo_lati_,
				double geo_longi_,
				const std::string& youtube_video_id_,
				const std::string& topics_);

		Entry(std::ifstream& ifs);
	};

	void Load();

	void StatNumReqsByPop();

	void FreeMem();
};


std::ostream& operator<< (std::ostream& os, const Ops::Entry& op);
std::ostream& operator<< (std::ostream& os, const Ops::Type& opt);
