#pragma once

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>

class DC;

namespace Ops {
	struct Entry {
		boost::posix_time::ptime created_at;
		double geo_lati;
		double geo_longi;

		Entry(
				const std::string& created_at_,
				double geo_lati_,
				double geo_longi_);
	};

	void Load();
	// TODO
	//void CntIBCByDCsByTime();
	//void CntFCKByDCsByLoc();

	void FreeMem();
};


std::ostream& operator<< (std::ostream& os, const Ops::Entry& op);
