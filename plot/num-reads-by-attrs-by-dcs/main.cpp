#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <cppconn/exception.h>
#include "conf.h"
#include "dc.h"
#include "op.h"

using namespace std;


void PrintStackTrace() {
  void* array[50];
  size_t size = backtrace(array, 50);
  backtrace_symbols_fd(array, size, STDOUT_FILENO);
}


void on_signal(int sig) {
	cout << "Got signal: " << sig << endl;
	PrintStackTrace();
  exit(1);
}


int main(int argc, char* argv[]) {
	try {
		signal(SIGSEGV, on_signal);
		signal(SIGINT, on_signal);

		Conf::Init(argc, argv);
		DCs::LoadDCs();

		Ops::Load();
		Ops::CntNumReadsBy();
		// TODO
		//Plot::NumReadsBy();

		Ops::FreeMem();
		DCs::FreeMem();
		return 0;
	} catch (sql::SQLException &e) {
		cout << "SQLException: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode() << ", SQLState: " << e.getSQLState() << " )\n";
	}
	catch (const exception& e) {
		cerr << e.what() << "\n";
	}
	return 1;
}
