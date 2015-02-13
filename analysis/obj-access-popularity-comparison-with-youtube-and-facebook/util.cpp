#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "util.h"

using namespace std;


namespace Util {
	const string& HomeDir() {
		static string homedir;
		if (homedir.size() == 0) {
			struct passwd *pw = getpwuid(getuid());
			homedir = pw->pw_dir;
		}
		return homedir;
	}

	const string& SrcDir() {
		static const string dn = boost::filesystem::path(__FILE__).parent_path().string();
		return dn;
	}

	string ToYMD_HMS(const boost::posix_time::time_duration& td) {
		// in: 7027:33:44
		// out: 1y0m3d-19:33:44
		static const auto sep = boost::is_any_of(":");
		vector<string> t;
		string s = boost::posix_time::to_simple_string(td);
		boost::split(t, s, sep);

		int h = atoi(t[0].c_str());	// total hours
		int year = h / (365 * 24);	// starts with 0
		h -= (year * (365 * 24));	// remaining hours
		const double hours_in_month = 365.0/12*24;
		int month = int(h / hours_in_month);	// starts with 0
		h -= int(month * hours_in_month);
		int day = h / 24;
		h -= (day * 24);
		int hour = h;

		string out;

		bool started = false;
		if (year == 0 && started == false)
			;
		else {
			out += str(boost::format("%dy") % year);
			started = true;
		}

		if (month == 0 && started == false)
			;
		else {
			out += str(boost::format("%dm") % month);
			started = true;
		}

		if (day == 0 && started == false)
			;
		else {
			out += str(boost::format("%dd-") % day);
			started = true;
		}

		out += str(boost::format("%02d:%s:%02d") % hour % t[1] % atoi(t[2].c_str()));
		return out;
	}

	string Indent(const string& in, int indent) {
		//cout << "[" << in << "]\n";
		static const auto sep = boost::is_any_of("\n");

		string leading_spaces;
		for (int i = 0; i < indent; i ++) 
			leading_spaces += " ";

		vector<string> tokens;
		boost::split(tokens, in, sep);
		string out;

		for (size_t i = 0; i < tokens.size(); i ++) {
			if (i != 0)
				out += "\n";
			if (tokens[i].size() != 0)
				out += (leading_spaces + tokens[i]);
		}   

		return out;
	}

	string Prepend(const string& p, const string& in) {
		static const auto sep = boost::is_any_of("\n");
		vector<string> tokens;
		boost::split(tokens, in, sep);
		string out;

		for (size_t i = 0; i < tokens.size(); i ++) {
			if ((i == tokens.size() - 1) && tokens[i].size() == 0)
				continue;
			out += (p + tokens[i] + "\n");
		}   
		return out;
	}

	string RunSubprocess(const string& cmd_) {
		string output;
		string cmd = string("( ") + cmd_ + " ) 2>&1";
		FILE* pipe = popen(cmd.c_str(), "r");
		if (! pipe)
			throw runtime_error(boost::str(boost::format("Unable to popen: %1%") % cmd));

		char buffer[4096];
		while (! feof(pipe)) {
			if (fgets(buffer, sizeof(buffer), pipe))
				output += buffer;
		}
		pclose(pipe);
		return output;
	}

	void SetEnv(const char* k, const char* v) {
		if (setenv(k, v, 1) != 0)
			throw runtime_error(str(boost::format("Unable to setenv: %s %s") % k % v));
	}

	void SetEnv(const char* k, const string& v) {
		if (setenv(k, v.c_str(), 1) != 0)
			throw runtime_error(str(boost::format("Unable to setenv: %s %s") % k % v));
	}

	void ClearLine() {
		// http://en.wikipedia.org/wiki/ANSI_escape_code
		//static const String ESC = "\033[";
		#define ESC "\033["
		cout << ESC "1K"	// clear from cursor to beginning of the line
			ESC "1G"; // move the cursor to column 1
	}

	// http://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c
	string ExecGetOutput(const string& cmd) {
		FILE* pipe = popen(cmd.c_str(), "r");
		if (!pipe)
			throw runtime_error(str(boost::format("error executing: %1%") % cmd));
		char buffer[1024];
		string result = "";
		while(!feof(pipe)) {
			if(fgets(buffer, sizeof(buffer), pipe) != NULL)
				result += buffer;
		}
		pclose(pipe);
		return result;
	}

	CpuTimer::CpuTimer(const string& msg, int indent) {
		_tmr = new boost::timer::cpu_timer();
		_indent = indent;
		for (int i = 0; i < _indent; i ++)
			cout << " ";
		cout << msg;
	}

	CpuTimer::~CpuTimer() {
		for (int i = 0; i < _indent + 2; i ++)
			cout << " ";
		cout << _tmr->elapsed().wall / 1000000000.0 << " sec.\n";
		delete _tmr;
	}
};
