#pragma once

#include <iostream>
#include <string>
#include "op.h"

struct OpYoutube : public Op {
	long id;	// tweet id
	long uid;

	// Op::obj_id is youtube_video_id
	
	long youtube_video_uploader;
	std::vector<std::string> topics;

	OpYoutube(std::ifstream& ifs);
	void Write(std::ofstream& ofs);
	std::string to_string() const;
};

std::ostream& operator<< (std::ostream& os, const OpYoutube& op);
