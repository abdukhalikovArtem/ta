#pragma once

#include "../../../fcs.h"
#include <fstream>

class ObjectDump {
private:
	std::ofstream file;
	int frameCount;
public:
	ObjectDump(std::string filename) { file.open(filename); frameCount = 0;  };
	~ObjectDump() { file.close(); };

	void Dump(fcs::object o) {
		file << " frame : " << frameCount << " x0 " << o.bbox.x << " y0 " << o.bbox.y << " width " << o.bbox.width << " height " << o.bbox.height << std::endl;
		//file << " points : ";
		//for (auto& a : o.segment)
		//	file << "( " << a.x << "; " << a.y << ")";
		file << std::endl; 
		frameCount++;
	}

	void Dump(std::vector<fcs::TFeature>& feat) {
		for (auto a : feat)
			file << frameCount << "\t" << a.x << "\t" << a.y << "\t" << a.size << "\t" << a.majority << std::endl;
		frameCount++;
		std::cout << frameCount << std::endl;
	};

	void Dump(std::vector<fcs::object>& o) {
		for (auto a : o)
			file << frameCount << "\t" << a.mainFeat.x << "\t" << a.mainFeat.y << "\t" << a.mainFeat.size << "\t" << a.mainFeat.majority << std::endl;
		frameCount++;
		std::cout << frameCount << std::endl;
	};
};