#ifndef __UTIL_H__
#define __UTIL_H__

#include <fstream>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/timer/timer.hpp>

namespace Util {
	const std::string& HomeDir();

	const std::string& SrcDir();

	std::string ToYMD_HMS(const boost::posix_time::time_duration& td);

	std::string Indent(const std::string& in, int indent);
	std::string Prepend(const std::string& in, const std::string& p);

	void ClearLine();

	void SetEnv(const std::string& k, const std::string& v);
	std::string ExecGetOutput(const std::string& cmd);

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
