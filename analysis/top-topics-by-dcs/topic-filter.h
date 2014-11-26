#ifndef __TOPIC_FILTER_H__
#define __TOPIC_FILTER_H__

#include <string>

namespace TopicFilter {
	void Load();
	bool IsBlackListed(const std::string& t);
};

#endif
