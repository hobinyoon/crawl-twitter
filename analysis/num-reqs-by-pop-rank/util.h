#pragma once

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/timer/timer.hpp>

namespace Util {
	const std::string& HomeDir();

	const std::string& SrcDir();

	std::string ToYMD_HMS(const boost::posix_time::time_duration& td);

	std::string Indent(const std::string& in, int indent);
	std::string Prepend(const std::string& p, const std::string& in);

	std::string RunSubprocess(const std::string& cmd_);
	void SetEnv(const char* k, const char* v);
	void SetEnv(const char* k, const std::string& v);

	void ClearLine();

	std::string ExecGetOutput(const std::string& cmd);

	struct CpuTimer {
		boost::timer::cpu_timer* _tmr;
		int _indent;
		CpuTimer(const std::string& msg, int indent = 0);
		~CpuTimer();
	};
};
