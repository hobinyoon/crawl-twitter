#include "review.h"
#include "util.h"

using namespace std;

Review::Review(ifstream& ifs) {
	ifs.read((char*)&votes, sizeof(votes));
	Util::ReadStr(ifs, user_id);
	Util::ReadStr(ifs, review_id);
	ifs.read((char*)&stars, sizeof(stars));
	Util::ReadStr(ifs, date);
	Util::ReadStr(ifs, text);
	Util::ReadStr(ifs, business_id);
}

ostream& operator << (ostream& os, const Review& r) {
	for (int i = 0; i < 3; i ++)
		os << r.votes[i] << " ";
	os << "\n"
		<< r.user_id << "\n"
		<< r.review_id << "\n"
		<< r.stars << "\n"
		<< r.date << "\n"
		<< r.text << "\n"
		<< r.business_id << "\n";
	return os;
}
