#include "SolvedFilterGenerator.h"
#include <cmath>
#include "opencv2\opencv.hpp"

SolvedFilterGenerator::SolvedFilterGenerator() {
	imgBufSize = 0;
}

SolvedFilterGenerator::~SolvedFilterGenerator() {
	if (imgBufSize) {
		delete[] imgIntBuffer;
	}
}

float SolvedFilterGenerator::Gexp(float x, float sigma) {
	return exp(- x * x / (2 * sigma * sigma));
}

SolvedFilterGenerator::solvedFilter SolvedFilterGenerator::LoGFilter(LoGSettings settings) {
	int filterSize = settings.filterSize;
	filterSize = (filterSize % 2) ? filterSize : filterSize + 1;

	std::vector<double> row;
	std::vector<double> col;

	double sigma = settings.sigma;

	for (int i = 0; i < filterSize; i++) {
		int diff = (filterSize - 1) / 2;
		double a = i - diff;
		double buf1 =  1 / (sqrt(2 * pi) * pow(sigma, 2.0)) * (variable - a * a / (sigma * sigma)) * Gexp(a, sigma);
		double buf2 =  1 / (sqrt(2 * pi) * pow(sigma, 2.0)) * Gexp(a, sigma);

		row.push_back(buf1);
		col.push_back(buf2);
	}
		
	solvedFilter logFilter;
	logFilter.col.push_back(row);
	logFilter.col.push_back(col);
	logFilter.row.push_back(col);
	logFilter.row.push_back(row);
	logFilter.size = filterSize;
	return logFilter;
}

SolvedFilterGenerator::filter  SolvedFilterGenerator::GenerateFilter(LoGSettings settings) {
	solvedFilter filter = LoGFilter(settings);
	SolvedFilterGenerator::filter genFilter;

	double pixelSum = 0;
	double pixelAbsSum = 0;
	double normK = 0;
	double max, min;
	double error = 0;

	//Allocate memory if nessesary
	if (imgBufSize < filter.size * filter.size) {
		if (imgBufSize) {
			delete[] imgIntBuffer;
		}
		imgBufSize = filter.size * filter.size;
		imgIntBuffer = new long long int[imgBufSize];
	}

	//Clean memory
	for (int i = 0; i < filter.size; i++)
		for (int j = 0; j < filter.size; j++) {
			imgIntBuffer[i * filter.size + j] = 0;
		}

	//Generate max min stat
	for (int k = 0; k < filter.row.size(); k++)
		for (int i = 0; i < filter.size; i++) {
			double pixel1 = abs(filter.row[k][i]);
			double pixel2 = abs(filter.col[k][i]);
			double lmax, lmin;
			lmax = std::max(pixel1, pixel2);
			lmin = std::min(pixel1, pixel2);
			if ((i == 0) && (k == 0)) {
				max = lmax;
				min = lmin;
			} else {
				max = (lmax > max ) ? lmax : max;
				min = (lmin < min ) ? lmin : min;
			}	
		}

	//generate integer filter
	double bufMult = 1 / max * MAX_FILTER_VAL;
	for (int k = 0; k < filter.row.size(); k++) 
		for (int i = 0; i < filter.size; i++)
			for (int j = 0; j < filter.size; j++) {
				int row = bufMult * filter.row[k][i];
				int col = bufMult * filter.col[k][j];
				imgIntBuffer[i * filter.size + j] += row * col;
			}

	//generate integer filter statistic
	for (int i = 0; i < filter.size; i++)
		for (int j = 0; j < filter.size; j++) {
			long long int pixel = (imgIntBuffer[i * filter.size + j]);
			pixelAbsSum += abs(pixel);
			pixelSum += pixel;
		}

	//Pack filter to struct
	/*for (int k = 0; k < filter.row.size(); k++) {
		std::vector<short> rowBuf, colBuf;
		for (int i = 0; i < filter.size; i++) {
			int row = bufMult * filter.row[k][i];
			int col = bufMult * filter.col[k][i];
			rowBuf.push_back(row);
			colBuf.push_back(col);
		}
		genFilter.col.push_back(colBuf);
		genFilter.row.push_back(rowBuf);
	}*/

	genFilter.filterSize = filter.row[0].size();
	genFilter.filterNorm = pixelAbsSum;
	genFilter.filterError = pixelSum;
	genFilter.filterNormError = pixelSum / pixelAbsSum;
	genFilter.dcol = filter.col;
	genFilter.drow = filter.row;

	return genFilter;
}

SolvedFilterGenerator::filter SolvedFilterGenerator::GenerateLogDilter(LoGSettings settings) {
	double maxError = 0.0001;
	double left = 0.5;
	double right = 1;
	SolvedFilterGenerator::filter filter;

	double fCent;
	double center;

	for (int i = 0; i < 1000; i++) {
		center = (right + left) / 2;
		variable = center;
		filter = GenerateFilter(settings);
		fCent = filter.filterNormError;

		variable = right;
		filter = GenerateFilter(settings);
		double fRight = filter.filterNormError;

		variable = left;
		filter = GenerateFilter(settings);
		double fLeft = filter.filterNormError;

		if ((fLeft < 0) && (fCent > 0)) {
			left = left;
			right = center;
		}
		else {
			left = center;
			right = right;
		}

		if (abs(fCent) < maxError)
			break;
		
		//std::cout << "i" << i <<  std::endl;
	} ;
	std::cout << "filter_done" << std::endl;
	variable = center;
	filter = GenerateFilter(settings);
	fCent = filter.filterError;

	return filter;
}