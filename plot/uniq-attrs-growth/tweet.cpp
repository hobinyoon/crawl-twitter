#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include "dc.h"
#include "tweet.h"

using namespace std;

Tweet::Tweet(long id_,
		long uid_,
		const string& created_at_,
		double geo_lati_,
		double geo_longi_,
		const string& youtube_video_id_,
		const string& topics_)
: id(id_), uid(uid_), created_at_str(created_at_),
	geo_lati(geo_lati_), geo_longi(geo_longi_),
	youtube_video_id(youtube_video_id_)
{
	created_at = boost::posix_time::time_from_string(created_at_str);

	static const auto sep = boost::is_any_of(" ");
	boost::split(topics, topics_, sep);
}


DC* Tweet::LocalDC() {
	return DCs::GetClosest(geo_lati, geo_longi);
}


std::ostream& operator<< (std::ostream& os, const Tweet& u) {
	os << boost::format("%ld %10ld %s %s %10lf %11lf %s")
		% u.id
		% u.uid
		% u.created_at
		% u.type
		% u.geo_lati
		% u.geo_longi
		% u.youtube_video_id;
	return os;
}


std::ostream& operator<< (std::ostream& os, const Tweet::Type& t) {
	static const char* s[] = {"W", "R"};
	os << s[t];
	return os;
}
