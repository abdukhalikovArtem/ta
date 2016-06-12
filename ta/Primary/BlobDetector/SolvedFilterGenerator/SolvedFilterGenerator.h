#pragma once
#include "../../../../fcs.h"

class SolvedFilterGenerator {
public:
	struct filter {
		short filterSize;
		long long int filterNorm;
		long long int filterError;
		double filterNormError;
		std::vector<std::vector<double>> drow;
		std::vector<std::vector<double>> dcol;
	};
	struct LoGSettings {
		float sigma;
		int filterSize;
	};
	SolvedFilterGenerator();
	~SolvedFilterGenerator();

	
	filter GenerateLogDilter (LoGSettings settings);

private:
	float Gexp(float x, float sigma);
	const double pi = 3.14159265358979323846;
	
	double variable;

	const int MAX_FILTER_VAL = 1024;
	struct solvedFilter {
		short size;
		std::vector<std::vector<double>> row;
		std::vector<std::vector<double>> col;
	};
	
	long long int *imgIntBuffer;
	double* imgDoubleBuf;
	int imgBufSize;

	solvedFilter LoGFilter(LoGSettings settings);
	filter GenerateFilter(LoGSettings settings);
	
};