#include <iostream>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "dc.h"
#include "conf.h"
#include "util.h"

using namespace std;


DC::DC(
		const string& name_,
		double lati_,
		double longi_)
	: name(name_), lati(lati_), longi(longi_)
{ }


namespace DCs {
	static vector<DC*> _entries;

	// http://en.wikipedia.org/wiki/Haversine_formula
	// http://blog.julien.cayzac.name/2008/10/arc-and-distance-between-two-points-on.html

	/// @brief The usual PI/180 constant
	static const double DEG_TO_RAD = 0.017453292519943295769236907684886;
	/// @brief Earth's quatratic mean radius for WGS-84
	static const double EARTH_RADIUS_IN_METERS = 6372797.560856;

	/** @brief Computes the arc, in radian, between two WGS-84 positions.
	 *
	 * The result is equal to <code>Distance(from,to)/EARTH_RADIUS_IN_METERS</code>
	 *    <code>= 2*asin(sqrt(h(d/EARTH_RADIUS_IN_METERS )))</code>
	 *
	 * where:<ul>
	 *    <li>d is the distance in meters between 'from' and 'to' positions.</li>
	 *    <li>h is the haversine function: <code>h(x)=sin²(x/2)</code></li>
	 * </ul>
	 *
	 * The haversine formula gives:
	 *    <code>h(d/R) = h(from.lat-to.lat)+h(from.lon-to.lon)+cos(from.lat)*cos(to.lat)</code>
	 *
	 * @sa http://en.wikipedia.org/wiki/Law_of_haversines
	 */
	static double ArcInRadians(
			double lat0, double lon0,
			double lat1, double lon1) {
		double latitudeArc  = (lat0 - lat1) * DEG_TO_RAD;
		double longitudeArc = (lon0 - lon1) * DEG_TO_RAD;
		double latitudeH = sin(latitudeArc * 0.5);
		latitudeH *= latitudeH;
		double lontitudeH = sin(longitudeArc * 0.5);
		lontitudeH *= lontitudeH;
		double tmp = cos(lat0 * DEG_TO_RAD) * cos(lat1 * DEG_TO_RAD);
		return 2.0 * asin(sqrt(latitudeH + tmp*lontitudeH));
	}


	void LoadDCs() {
		const string& fn = Conf::fn_dc_coord;
		cout << "Loading DCs from file " << fn << " max_dcs=" << Conf::max_dcs << " ...\n";

		ifstream ifs(fn.c_str());
		if (! ifs.is_open())
			throw runtime_error(str(boost::format("Unable to open file %1%") % fn));

		string line;
		while (getline(ifs, line)) {
			if (line.length() == 0)
				continue;
			if (line[0] == '#')
				continue;
			vector<string> t;
			boost::split(t, line, boost::is_any_of(" ,"));
			if (t.size() != 3 && t.size() != 4)
				continue;
			//cout << line << endl;
			_entries.push_back(new DC(t[0], atof(t[1].c_str()), atof(t[2].c_str())));
			if (_entries.size() == Conf::max_dcs)
				break;
		}

		sort(_entries.begin(), _entries.end(),
			[] (const DC* a, const DC* b) {
				return (a->name < b->name);
			});
		cout << "  Loaded " << _entries.size() << " DC(s)\n";
	}


	void FreeMem() {
		for (auto e: _entries)
			delete e;
		_entries.clear();
	}


	DC* GetClosest(double lati, double longi) {
		DC* c_dc = NULL;
		double min_dist = 0.0;

		for (auto dc: _entries) {
			double dist = ArcInRadians(lati, longi, dc->lati, dc->longi);
			if (c_dc == NULL) {
				c_dc = dc;
				min_dist = dist;
			} else {
				if (min_dist > dist) {
					c_dc = dc;
					min_dist = dist;
				}
			}
		}
		return c_dc;
	}


	const vector<DC*>& GetAll() {
		return _entries;
	}
}
