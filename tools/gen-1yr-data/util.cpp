#include <libgen.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <boost/algorithm/string.hpp>
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

	void ClearLine() {
		// http://en.wikipedia.org/wiki/ANSI_escape_code
		//static const String ESC = "\033[";
		#define ESC "\033["
		cout << ESC "1K"	// clear from cursor to beginning of the line
			ESC "1G"; // move the cursor to column 1
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

	const string& SrcDir() {
		static string srcdir;
		if (srcdir.size() == 0) {
			char file[PATH_MAX];
			strcpy(file, __FILE__);
			srcdir = dirname(file);
		}
		return srcdir;
	}

	void ReadBinaryFile(const string& fn, string& s) {
		ifstream ifs(fn, std::ios::in | std::ios::binary);
		if (! ifs.is_open())
			throw runtime_error(str(boost::format("unable to open file %1%") % fn));
		ifs.seekg(0, ios::end);
		s.reserve(ifs.tellg());
		ifs.seekg(0, ios::beg);
		s.assign((istreambuf_iterator<char>(ifs)),
				istreambuf_iterator<char>());
	}

	void ReadStr(ifstream& ifs, string& str) {
		size_t s;
		ifs.read((char*)&s, sizeof(s));
		str.resize(s);
		ifs.read((char*)&str[0], s);
	}

	void WriteStr(ofstream& ofs, const string& str) {
		size_t s = str.size();
		ofs.write((char*)&s, sizeof(s));
		ofs.write(str.c_str(), str.size());
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
