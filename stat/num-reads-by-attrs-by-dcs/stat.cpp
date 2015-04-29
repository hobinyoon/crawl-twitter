#include <fstream>
#include <boost/format.hpp>
#include "stat.h"
#include "util.h"

using namespace std;

namespace Stat {

	void AvgMinMax(
			const vector<double>& values,
			double& avg,
			double& min,
			double& max) {
		size_t v_size = values.size();
		if (v_size == 0)
			throw runtime_error("No elements in vector for average");
		double s = 0.0;
		for (size_t i = 0; i < v_size; i ++) {
			if (i == 0) {
				min = max = values[i];
			} else {
				min = std::min(min, values[i]);
				max = std::max(max, values[i]);
			}
			s += values[i];
		}
		avg = s / v_size;
	}

	// note that the input values are modified.
	void CDF(
			vector<size_t>& values,
			double* x,
			double* y,
			bool print_result,
			int indent) {
		size_t sum = 0;
		size_t min = -1, max = -1;
		size_t v_len = values.size();

		for (size_t i = 0; i < v_len; i ++) {
			if (i == 0) {
				min = max = values[i];
			} else {
				min = std::min(min, values[i]);
				max = std::max(max, values[i]);
			}
			sum += values[i];
		}

		double avg = double(sum) / v_len;

		sort(values.begin(), values.end());

		// percentile
		const double pct_y[] = {0.01, 0.05, 0.10, 0.50, 0.90, 0.95, 0.99};
		bool pct_x_set[] = {false, false, false, false, false, false, false};
		double pct_min_y = -1;
		double avg_y = -1;
		bool avg_y_set = false;
		size_t pct_x[7];

		for (size_t i = 0; i < v_len; i ++) {
			if (i == 0)
				pct_min_y = (double) (i + 1) / v_len;

			if ((avg_y_set == false) && (avg <= values[i])) {
				avg_y = (double) (i + 1) / v_len;
				avg_y_set = true;
			}

			for (size_t j = 0; j < 7; j ++ ) {
				if ((pct_x_set[j] == false) && (i / pct_y[j] >= v_len)) {
					pct_x[j] = values[i];
					pct_x_set[j] = true;
				}
			}
		}

		// avg, min, max, and percentiles
		x[0] = avg;
		x[1] = min;
		x[2] = max;
		for (size_t i = 0; i < 7; i ++)
			x[i + 3] = pct_x[i];

		y[0] = avg_y;
		y[1] = pct_min_y;
		y[2] = 1.0;
		for (size_t i = 0; i < 7; i ++)
			y[i + 3] = pct_y[i];

		if (print_result) {
			cout << Util::Indent(
					str(boost::format(
							"num: %8d\n"
							"avg: %8f %f\n"
							"min: %8d %f\n"
							"max: %8d %f\n"
							" 1p: %8d %f\n"
							" 5p: %8d %f\n"
							"10p: %8d %f\n"
							"50p: %8d %f\n"
							"90p: %8d %f\n"
							"95p: %8d %f\n"
							"99p: %8d %f\n")
						% v_len
						% x[0] % y[0]
						% x[1] % y[1]
						% x[2] % y[2]
						% x[3] % y[3]
						% x[4] % y[4]
						% x[5] % y[5]
						% x[6] % y[6]
						% x[7] % y[7]
						% x[8] % y[8]
						% x[9] % y[9]), indent);
		}
	}


	// note that the input values are modified.
	void CDF(
			vector<double>& values,
			double* x,
			double* y) {
		double sum = 0.0;
		double min = -1, max = -1;
		size_t tds_size = values.size();

		for (size_t i = 0; i < tds_size; i ++) {
			if (i == 0) {
				min = max = values[i];
			} else {
				min = std::min(min, values[i]);
				max = std::max(max, values[i]);
			}
			sum += values[i];
		}

		auto avg = sum / tds_size;

		sort(values.begin(), values.end());

		// percentile
		const double pct_y[] = {0.01, 0.05, 0.10, 0.50, 0.90, 0.95, 0.99};
		bool pct_x_set[] = {false, false, false, false, false, false, false};
		double pct_min_y = -1;
		double avg_y = -1;
		bool avg_y_set = false;
		double pct_x[7];

		for (size_t i = 0; i < tds_size; i ++) {
			if (i == 0)
				pct_min_y = (double) (i + 1) / tds_size;

			if ((avg_y_set == false) && (avg <= values[i])) {
				avg_y = (double) (i + 1) / tds_size;
				avg_y_set = true;
			}

			for (size_t j = 0; j < 7; j ++ ) {
				if ((pct_x_set[j] == false) && (i / pct_y[j] >= tds_size)) {
					pct_x[j] = values[i];
					pct_x_set[j] = true;
				}
			}
		}

		// avg, min, max, and percentiles
		x[0] = avg;
		x[1] = min;
		x[2] = max;
		for (size_t i = 0; i < 7; i ++)
			x[i + 3] = pct_x[i];

		y[0] = avg_y;
		y[1] = pct_min_y;
		y[2] = 1.0;
		for (size_t i = 0; i < 7; i ++)
			y[i + 3] = pct_y[i];
	}


	// note that the input values are modified.
	void CDF(
			vector<boost::posix_time::time_duration>& tds,
			boost::posix_time::time_duration* x,
			double* y) {
		boost::posix_time::time_duration sum;
		boost::posix_time::time_duration min, max;
		size_t tds_size = tds.size();

		for (size_t i = 0; i < tds_size; i ++) {
			if (i == 0) {
				min = max = tds[i];
			} else {
				min = std::min(min, tds[i]);
				max = std::max(max, tds[i]);
			}
			sum += tds[i];
		}

		auto avg = sum / tds_size;

		sort(tds.begin(), tds.end());

		// percentile
		const double pct_y[] = {0.01, 0.05, 0.10, 0.50, 0.90, 0.95, 0.99};
		bool pct_x_set[] = {false, false, false, false, false, false, false};
		double pct_min_y = -1;
		double avg_y = -1;
		bool avg_y_set = false;
		boost::posix_time::time_duration pct_x[7];

		for (size_t i = 0; i < tds_size; i ++) {
			if (i == 0)
				pct_min_y = (double) (i + 1) / tds_size;

			if ((avg_y_set == false) && (avg <= tds[i])) {
				avg_y = (double) (i + 1) / tds_size;
				avg_y_set = true;
			}

			for (size_t j = 0; j < 7; j ++ ) {
				if ((pct_x_set[j] == false) && (i / pct_y[j] >= tds_size)) {
					pct_x[j] = tds[i];
					pct_x_set[j] = true;
				}
			}
		}

		// avg, min, max, and percentiles
		x[0] = avg;
		x[1] = min;
		x[2] = max;
		for (size_t i = 0; i < 7; i ++)
			x[i + 3] = pct_x[i];

		y[0] = avg_y;
		y[1] = pct_min_y;
		y[2] = 1.0;
		for (size_t i = 0; i < 7; i ++)
			y[i + 3] = pct_y[i];

		cout << "  stat:\n";
		for (size_t i = 0; i < 10; i ++)
			cout << (boost::format("  %17s %10f") % Util::ToYMD_HMS(x[i]) % y[i]) << "\n";
	}

	void GenCDFPlotData(vector<size_t>& values, const string& fn) {
		ofstream ofs(fn);
		if (! ofs.is_open())
			throw runtime_error(str(boost::format("unable to open file %1%") % fn));
		ofs << std::fixed;
		size_t values_len = values.size();
		bool first = true;
		double prev_x;
		for (size_t i = 0; i < values_len; i ++) {
			double y = double(i + 1) / values_len;
			if (first || (prev_x != values[i]))
				ofs << values[i] << " " << y << "\n";
			if (first)
				first = false;
			prev_x = values[i];
		}
	}
};
