#pragma once
//#include "ap_fixed.h"
#define SIMULATION
#define NUM_GAUSS 2
#define MAXINDEX (3*1024*768)
#define MAX_SIZE (1024*768)
#include <iostream>

#ifndef SIMULATION
#include "ap_fixed.h"
typedef short TIndex;
typedef ap_uint<12> TIntensity;
typedef char TGaussIndex;
typedef ap_ufixed<24,1> TWeight;
typedef ap_ufixed<24,16> TVar;
typedef ap_uint<24> TDist;
//typedef ap_fixed<25,1> Tfixedbuffer;
#else
typedef short TIndex;
typedef char TGaussIndex;
typedef short TIntensity;
typedef float TWeight;
typedef float TVar;
typedef int TDist;
#endif
/*
typedef short TIndex;
typedef ap_uint<16> TIntensity;
typedef ap_uint<2> TGaussIndex;
typedef ap_ufixed<16,16, AP_RND, AP_SAT> TWeight;
typedef ap_ufixed<16,16, AP_RND, AP_SAT> TVar;
typedef ap_uint<32> TDist;
*/


struct mogPixel{
	float  Weight[NUM_GAUSS];
	int    Mean[NUM_GAUSS];
	float   Variance[NUM_GAUSS];
};

/*
struct mogPixel{
    float  Weight[NUM_GAUSS];
    short  Mean[NUM_GAUSS];
    float    Variance[NUM_GAUSS];
};
*/

struct outData{
	mogPixel memory;
	TIndex address;
	bool bg;
};


struct inData{
	mogPixel memory;
	TIntensity pixel;
	TIndex address;
};

typedef struct _engIn{
	short pixel;
	int address;
} engIn;

class MogPixel{
	static const TVar varInit;
	static const TVar varMin;
	static const TVar varMax;
	static const TDist Tg;
	static const TWeight alphaT;
    static const TWeight BackgroundThreshold;

    TWeight Weight[NUM_GAUSS];
    TIntensity  Mean[NUM_GAUSS];
	TVar    Variance[NUM_GAUSS];

    void Swap(TGaussIndex k1, TGaussIndex k2);

    TIntensity GetIntensity() const;

    bool Process(TIntensity newPixelValue);

public:
    //void Init (void);
    outData UpdatePixel(inData data);
};
