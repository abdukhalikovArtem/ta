#include "MogPixel.h"

#include "stdlib.h"
using namespace std;
//const int MogPixel::NUM_GAUSS = 3;

const TWeight MogPixel::alphaT = 0.002;
const TVar MogPixel::varInit = 500;
const TVar MogPixel::varMin = 4;
const TVar MogPixel::varMax = 4 * varInit;
const TDist MogPixel::Tg = 5;

const TWeight MogPixel::BackgroundThreshold = 0.9;
//const TVar MogPixel::Variance = varInit;

void MogPixel::Swap(TGaussIndex k1, TGaussIndex k2){

	TVar buffer_var;
	TIntensity buffer_mean;
	TWeight buffer_weight;
	///weight
	buffer_weight = Weight[k1];
	Weight[k1] = Weight[k2];
	Weight[k2] = buffer_weight;
	///Mean
	buffer_mean = Mean[k1];
	Mean[k1] = Mean[k2];
	Mean[k2] = buffer_mean;
	///Variance
	buffer_var = Variance[k1];
	Variance[k1] = Variance[k2];
	Variance[k2] = buffer_var;
};

bool MogPixel::Process(TIntensity newPixelValue){
	bool backgound = 0;
	/// initialize model update mechanism

	TWeight totalWeight = 0;
	bool fitsPDF = 0;
	TGaussIndex kmode = 0;
	TDist meanDelta = 0;
	TDist meanDeltaSquare = 0;
Process_label0:for (TGaussIndex mode = 0; mode < NUM_GAUSS; mode++){
	kmode = mode;

	TDist dist2;// = 0;
	TDist dData;// = 0;
	TIntensity mean = Mean[mode];
	TVar var = Variance [mode];
	TWeight weight = Weight[mode];
	//TDist =
	bool o = 0;
	///fit not found yet

	if (!fitsPDF){
		if (weight == 0)///need to be initialized
			break;

		///check if it belongs to some of the remaining modes

		///calculate difference and distance

		dData = newPixelValue - mean;
		dist2 = dData*dData;
		TDist varThreshold = Tg*var;
		//cout << "varThreshold " << varThreshold << endl;
		//TDist distThreshold = Tg*var;
		if ((totalWeight < 0.9) && (dist2 < varThreshold))
			backgound = 1;
		///check fit
		if (dist2 < Tg*var){
			///belongs to the mode
			meanDelta = dData;
			meanDeltaSquare = dist2;
			fitsPDF = 1;
			o = 1;
		}
	}
	///update distribution

	///update weight
	weight += alphaT*(o - weight);

	totalWeight += weight;

	///update mean
	mean = mean + o*alphaT*meanDelta / weight;

	///update variance
	TVar varnew = var + o*alphaT*(meanDeltaSquare - var) / weight;


	///limit the variance
	varnew = (varnew >= varMin) ? varnew : varMin;
	varnew = (varnew <= varMax) ? varnew : varMax;

	///Push data to memory
	Variance[mode] = varnew;
	Mean[mode] = mean;
	Weight[mode] = weight;

	///Sort k - mode
	///sortModelParameters (i,j,kmode);
Process_label1:for (TGaussIndex ii = (NUM_GAUSS - 1); ii > 0; ii--){
	if (ii > mode)
		continue;
	///check one up
	if (weight < Weight[ii - 1])
		break;
	///swap one up
	Swap(ii, ii - 1);
}
}///for ended

			   ///INIT
			   if (!fitsPDF){
				   kmode = (NUM_GAUSS - 1);
				   Weight[kmode] = alphaT;
				   totalWeight += alphaT;
				   Mean[kmode] = newPixelValue;
				   Variance[kmode] = varInit;

			   label0: for (TGaussIndex k = 0; k < NUM_GAUSS; k++)
				   Weight[k] *= 1 - alphaT;

					   ///sortModelParameters (i,j,kmode);
				   Process_label2:for (TGaussIndex ii = (NUM_GAUSS - 1); ii > 0; ii--){
					   if (alphaT < Weight[ii - 1])
						   break;
					   Swap(ii, ii - 1);
				   }

			   }

			   
		   Process_label:for (TGaussIndex mode = 0; mode < NUM_GAUSS; mode++){
			   Weight[mode] /= totalWeight;
		   }

						 return backgound;
};


TIntensity MogPixel::GetIntensity() const {
	//TIntensity intensity = 0;
	TWeight intensity = 0;
	TWeight totalWeight = 0;
	for (TGaussIndex k = 0; k < NUM_GAUSS; k++){
		intensity += Weight[k] * Mean[k];
		totalWeight += Weight[k];
		if (totalWeight > BackgroundThreshold)
			break;
	}
	return (TIntensity)intensity;
}

outData MogPixel::UpdatePixel(inData data) {
	outData result;
	result.address = data.address;
UpdatePixel0:for (TGaussIndex mode = 0; mode < NUM_GAUSS; mode++){
	Weight[mode] = data.memory.Weight[mode];
	Mean[mode] = data.memory.Mean[mode];
	Variance[mode] = data.memory.Variance[mode];
}
			 result.bg = Process(data.pixel);
		 UpdatePixel1:for (int mode = 0; mode < NUM_GAUSS; mode++){
			 result.memory.Weight[mode] = Weight[mode];
			// cout << "Weight[" << mode << "] = " << Weight[mode] << endl;
			 result.memory.Mean[mode] = Mean[mode];
			// cout << "Mean[" << mode << "] = " << Mean[mode] << endl;
			 result.memory.Variance[mode] = Variance[mode];
			 //cout << "Variance[" << mode << "] = " << Variance[mode] << endl;
		 }
					//  cout << "BG " << result.bg << endl;
	return result;
};

