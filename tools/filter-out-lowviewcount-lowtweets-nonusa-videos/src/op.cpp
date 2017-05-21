#include "op.h"

using namespace std;

//bool operator< (const Op& a, const Op& b) {
//	return (a.obj_id < b.obj_id);
//}

Op::~Op()
{
}


void Op::SetCreatedAtPt() {
	created_at_pt = boost::posix_time::time_from_string(created_at);
}


ostream& operator<< (ostream& os, const Op::Type& opt) {
	static const char* s[] = {"NA", "W", "R"};
	os << s[opt];
	return os;
}
