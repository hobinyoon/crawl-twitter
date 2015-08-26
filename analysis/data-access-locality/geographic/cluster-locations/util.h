#pragma once

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/timer/timer.hpp>

namespace Util {
	const std::string& HomeDir();

	const std::string& SrcDir();

	std::string ToYMD_HMS(const boost::posix_time::time_duration& td);

	std::string Indent(const std::string& in, int indent);

	void ClearLine();

	std::string ExecGetOutput(const std::string& cmd);

	std::string ToStr(const std::string& s);
	std::string ToStr(const boost::basic_format<char>& bf);

	struct CpuTimer {
		boost::timer::cpu_timer* _tmr;
		int _indent;
		CpuTimer(const std::string& msg, int indent = 0);
		~CpuTimer();
	};
};

#define TRACE std::cout << basename(__FILE__) << " " << __LINE__ << " "

#define THROW_RE(msg) \
	throw std::runtime_error(str(boost::format("[%s %3d] %s") \
				% basename(__FILE__) % __LINE__ % Util::ToStr(msg)))
