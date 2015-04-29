#ifndef __STAT_H__
#define __STAT_H__

#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace Stat {
	void AvgMinMax(
			const std::vector<double>& values,
			double& avg,
			double& min,
			double& max);

	void CDF(
			std::vector<size_t>& values,
			double* x,
			double* y,
			bool print_result = false,
			int indent = 0);

	void CDF(
			std::vector<double>& values,
			double* x,
			double* y);

	void CDF(std::vector<boost::posix_time::time_duration>& tds,
			boost::posix_time::time_duration* x,
			double* y);

	void GenCDFPlotData(std::vector<size_t>& values, const std::string& fn);
};

#endif
