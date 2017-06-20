#pragma once

#include <iostream>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>

struct Op {
	enum Type {
		NA = 0,	// not assigned
		W = 1,
		R = 2,
	};

	Type type;
	double lat;
	double lon;
	std::string obj_id;

	// 2013-11-01 00:03:09
	std::string created_at;
	boost::posix_time::ptime created_at_pt;

	size_t size;
	char in_usa;

	virtual ~Op();

	void SetCreatedAtPt();

	virtual std::string to_string() const = 0;
};


// Needed by <set> or <map>
// bool operator< (const Op& a, const Op& b);


std::ostream& operator<< (std::ostream& os, const Op::Type& opt);
