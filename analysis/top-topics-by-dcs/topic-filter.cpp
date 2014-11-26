#include <fstream>
#include <set>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include "conf.h"
#include "topic-filter.h"
#include "util.h"

using namespace std;

namespace TopicFilter {
	set<string> _topics;

	void Load() {
		const string& fn = Conf::fn_topic_filter;
		cout << boost::format("Loading topic filter from file %s ...\n") % fn;

		ifstream ifs(fn);
		if (! ifs.is_open())
			throw runtime_error(str(boost::format("Unable to open file %s") % fn));

		auto sep = boost::is_any_of(" ");

		string line;
		while (getline(ifs, line)) {
			if (line.length() == 0)
				continue;
			if (line[0] == '#')
				continue;
			vector<string> t;
			boost::split(t, line, sep);
			if (t.size() != 2)
				throw runtime_error(str(boost::format("Unexpected format [%s]") % line));
			_topics.insert(t[0]);
		}

		cout << "  loaded " << _topics.size() << " topics\n";
	}

	bool IsBlackListed(const string& t) {
		if (_topics.find(t) == _topics.end()) {
			return false;
		} else {
			return true;
		}
	}
};
