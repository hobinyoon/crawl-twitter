#ifndef __DC_H__
#define __DC_H__

#include <string>
#include <vector>

class DC {
public:
	std::string name;
	double lati;
	double longi;

	DC(const std::string& name, double lati, double longi);
};


namespace DCs {
	void LoadDCs();
	void FreeMem();

	DC* GetClosest(double lati, double longi);
	const std::vector<DC*>& GetAll();
};

#endif
