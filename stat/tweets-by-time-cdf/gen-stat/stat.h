#ifndef __STAT_H__
#define __STAT_H__

#include <map>
#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace Stat {
	void AvgMinMax(
			const std::vector<double>& values,
			double& avg,
			double& min,
			double& max);

	void CDF(
			const std::map<size_t, int>& map_,
			double* x,
			double* y,
			bool print_result,
			int indent);

	void CDF(
			std::vector<double>& values,
			double* x,
			double* y,
			bool print_result,
			int indent);

	void CDF(std::vector<boost::posix_time::time_duration>& tds,
			boost::posix_time::time_duration* x,
			double* y);
};

#endif
