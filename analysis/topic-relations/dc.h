#pragma once

#include <string>
#include <vector>

struct DC {
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
