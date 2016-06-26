#ifndef __UTIL_H__
#define __UTIL_H__

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/timer/timer.hpp>

namespace Util {
	const std::string& HomeDir();

	std::string ToYMD_HMS(const boost::posix_time::time_duration& td);

	std::string Indent(const std::string& in, int indent);

	void ClearLine();

	std::string RunSubprocess(const std::string& cmd_);

	void SetEnv(const char* k, const char* v);
	void SetEnv(const char* k, const std::string& v);

	const std::string& SrcDir();

	void ReadBinaryFile(const std::string& fn, std::string& s);
	void ReadStr(std::ifstream& ifs, std::string& str);
	void WriteStr(std::ofstream& ofs, const std::string& str);

	struct CpuTimer {
		boost::timer::cpu_timer* _tmr;
		int _indent;
		CpuTimer(const std::string& msg, int indent = 0);
		~CpuTimer();
	};
};

#endif
