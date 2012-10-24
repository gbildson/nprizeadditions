/*
########################################################################
#  Netflix Prize Tools
#  Copyright (C) 2007-8 Ehud Ben-Reuven
#  udi@benreuven.com
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation version 2.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
########################################################################
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "basic.h"
#include "netflix.h"
#include "utest.h"
#include "weight.h"
static char *fnameV="data/usvdsppV.bin";
static char *fnameU="data/usvdsppU.bin";

#define E (0.00020) // stop condition
#define E2 (0.00002) // stop condition
int score_argv(char **argv) {return 0;}
extern double rmse(int k);



#define NFEATURES (10)
#define GLOBAL_MEAN (3.6033)
//double sU[NUSERS][NFEATURES];
double **sU;
//double sV[NMOVIES][NFEATURES];
//double sY[NMOVIES][NFEATURES];
double **sV;
double **sY;
double bU[NUSERS]; 
double bV[NMOVIES];
double bVbin[NMOVIES][30];
double alphabU[NUSERS];
//double alphasU[NUSERS][NFEATURES];
double **alphasU;


//#define G1 (0.007) 
//#define G2 (0.007) 
//#define L6 (0.005) // for biases
//#define L7 (0.015) 
//#define G4 (0.01) 
//#define L9 (0.006) // for biases


// GOOD!!
//#define G1 (0.0055) 
//#define G2 (0.0055) 
//#define L6 (0.004) // for biases
//#define L7 (0.012) 
//#define G4 (0.008) 
//#define L9 (0.005) // for biases

// BETTER!!!
//#define G1 (0.0045) 
//#define G2 (0.0045) 
//#define L6 (0.0034) // for biases
//#define L7 (0.010) 
////#define G4 (0.006) 
////#define L9 (0.004) // for biases

//#define G4abu  (0.006) 
//#define L9abu  (0.004) // for biases
//#define G4sdbu (0.006) 
//#define L9sdbu (0.004) // for biases
//#define G4bvb  (0.006) 
//#define L9bvb  (0.004) // for biases
//#define G4asu  (0.006) 
//#define L9asu  (0.004) // for biases
//#define G4sdsu (0.006) 
//#define L9sdsu (0.004) // for biases

// BETTER STILL
//#define G4abu  (0.006) 
//#define L9abu  (0.004) // for biases
//#define G4sdbu (0.006) 
//#define L9sdbu (0.004) // for biases
//#define G4bvb  (0.006) 
//#define L9bvb  (0.004) // for biases
//#define G4asu  (0.005) 
//#define L9asu  (0.0033) // for biases
//#define G4sdsu (0.006) 
//#define L9sdsu (0.004) // for biases

//Not so good
//#define G4abu  (0.006) 
//#define L9abu  (0.004) // for biases
//#define G4sdbu (0.006) 
//#define L9sdbu (0.004) // for biases
//#define G4bvb  (0.006) 
//#define L9bvb  (0.004) // for biases
//#define G4asu  (0.003) 
//#define L9asu  (0.0020) // for biases
//#define G4sdsu (0.006) 
//#define L9sdsu (0.004) // for biases

// Seems very good
//#define G4abu  (0.006) 
//#define L9abu  (0.004) // for biases
//#define G4sdbu (0.006) 
//#define L9sdbu (0.004) // for biases
//#define G4bvb  (0.006) 
//#define L9bvb  (0.004) // for biases
//#define G4asu  (0.001) 
//#define L9asu  (0.0007) // for biases
//#define G4sdsu (0.006) 
//#define L9sdsu (0.004) // for biases

//Good
//#define G4abu  (0.006) 
//#define L9abu  (0.004) // for biases
//#define G4sdbu (0.006) 
//#define L9sdbu (0.004) // for biases
//#define G4bvb  (0.006) 
//#define L9bvb  (0.004) // for biases
//#define G4asu  (0.00075) 
//#define L9asu  (0.00050) // for biases
//#define G4sdsu (0.006) 
//#define L9sdsu (0.004) // for biases

// Better 5
//#define G4abu  (0.006) 
//#define L9abu  (0.004) // for biases
//#define G4sdbu (0.006) 
//#define L9sdbu (0.004) // for biases
//#define G4bvb  (0.006) 
//#define L9bvb  (0.004) // for biases
//#define G4asu  (0.00075) 
//#define L9asu  (0.00050) // for biases
//#define G4sdsu (0.004) 
//#define L9sdsu (0.003) // for biases

// 8 Best so far 6
// RMSE Train 0.641967 (36.8%) Probe 0.898967 (7.0%) Both 0.646276 (36.1%)
//#define G4abu  (0.006) 
//#define L9abu  (0.004) // for biases
//#define G4sdbu (0.006) 
//#define L9sdbu (0.004) // for biases
//#define G4bvb  (0.006) 
//#define L9bvb  (0.004) // for biases
//#define G4asu  (0.00060) 
//#define L9asu  (0.00040) // for biases
//#define G4sdsu (0.003) 
//#define L9sdsu (0.002) // for biases

// 9 
// RMSE Train 0.664716 (32.1%) Probe 0.899246 (6.9%) Both 0.668572 (31.5%)
//#define G4abu  (0.006) 
//#define L9abu  (0.004) // for biases
//#define G4sdbu (0.006) 
//#define L9sdbu (0.004) // for biases
//#define G4bvb  (0.006) 
//#define L9bvb  (0.004) // for biases
//#define G4asu  (0.00045) 
//#define L9asu  (0.00030) // for biases
//#define G4sdsu (0.002) 
//#define L9sdsu (0.0014) // for biases

// 10 
// RMSE Train 0.643076 (36.5%) Probe 0.898983 (7.0%) Both 0.647362 (35.8%)
//#define G4abu  (0.006) 
//#define L9abu  (0.004) // for biases
//#define G4sdbu (0.006) 
//#define L9sdbu (0.004) // for biases
//#define G4bvb  (0.006) 
//#define L9bvb  (0.004) // for biases
//#define G4asu  (0.00045) 
//#define L9asu  (0.00030) // for biases
//#define G4sdsu (0.003) 
//#define L9sdsu (0.002) // for biases

// 11 
// RMSE Train 0.663588 (32.3%) Probe 0.899201 (6.9%) Both 0.667465 (31.7%)
//#define G4abu  (0.006) 
//#define L9abu  (0.004) // for biases
//#define G4sdbu (0.006) 
//#define L9sdbu (0.004) // for biases
//#define G4bvb  (0.006) 
//#define L9bvb  (0.004) // for biases
//#define G4asu  (0.00060) 
//#define L9asu  (0.00040) // for biases
//#define G4sdsu (0.002) 
//#define L9sdsu (0.0014) // for biases


// 12
// RMSE Train 0.642038 (36.8%) Probe 0.898855 (7.0%) Both 0.646343 (36.0%)
//#define G4abu  (0.006) 
//#define L9abu  (0.004) // for biases
//#define G4sdbu (0.006) 
//#define L9sdbu (0.004) // for biases
//#define G4bvb  (0.004) 
//#define L9bvb  (0.003) // for biases
//#define G4asu  (0.00060) 
//#define L9asu  (0.00040) // for biases
//#define G4sdsu (0.003) 
//#define L9sdsu (0.002) // for biases

// 13
// RMSE Train 0.644912 (36.1%) Probe 0.898994 (7.0%) Both 0.649161 (35.4%)
//#define G4abu  (0.006) 
//#define L9abu  (0.004) // for biases
//#define G4sdbu (0.004) 
//#define L9sdbu (0.003) // for biases
//#define G4bvb  (0.004) 
//#define L9bvb  (0.003) // for biases
//#define G4asu  (0.00060) 
//#define L9asu  (0.00040) // for biases
//#define G4sdsu (0.003) 
//#define L9sdsu (0.002) // for biases

// 14 
// RMSE Train 0.644869 (36.2%) Probe 0.898271 (7.1%) Both 0.649105 (35.5%)
//#define G4abu  (0.004) 
//#define L9abu  (0.003) // for biases
//#define G4sdbu (0.004) 
//#define L9sdbu (0.003) // for biases
//#define G4bvb  (0.004) 
//#define L9bvb  (0.003) // for biases
//#define G4asu  (0.00060) 
//#define L9asu  (0.00040) // for biases
//#define G4sdsu (0.003) 
//#define L9sdsu (0.002) // for biases

// 15 
// RMSE Train 0.638583 (37.5%) Probe 0.898260 (7.1%) Both 0.642948 (36.8%)
//#define G4abu  (0.004) 
//#define L9abu  (0.003) // for biases
//#define G4sdbu (0.009) 
//#define L9sdbu (0.006) // for biases
//#define G4bvb  (0.004) 
//#define L9bvb  (0.003) // for biases
//#define G4asu  (0.00060) 
//#define L9asu  (0.00040) // for biases
//#define G4sdsu (0.003) 
//#define L9sdsu (0.002) // for biases

// 16 
// RMSE Train 0.638698 (37.5%) Probe 0.898087 (7.1%) Both 0.643057 (36.7%)
//#define G4abu  (0.004) 
//#define L9abu  (0.003) // for biases
//#define G4sdbu (0.009) 
//#define L9sdbu (0.006) // for biases
//#define G4bvb  (0.002) 
//#define L9bvb  (0.00133) // for biases
//#define G4asu  (0.00060) 
//#define L9asu  (0.00040) // for biases
//#define G4sdsu (0.003) 
//#define L9sdsu (0.002) // for biases

// 17 Good
// RMSE Train 0.638707 (37.5%) Probe 0.897491 (7.1%) Both 0.643054 (36.7%)
//#define G4abu  (0.002) 
//#define L9abu  (0.00133) // for biases
//#define G4sdbu (0.009) 
//#define L9sdbu (0.006) // for biases
//#define G4bvb  (0.002) 
//#define L9bvb  (0.00133) // for biases
//#define G4asu  (0.00060) 
//#define L9asu  (0.00040) // for biases
//#define G4sdsu (0.003) 
//#define L9sdsu (0.002) // for biases

// 18 
// 0.635759        0.898842        176.240000
//#define G4abu  (0.001) 
//#define L9abu  (0.000666) // for biases
//#define G4sdbu (0.02) 
//#define L9sdbu (0.0133) // for biases
//#define G4bvb  (0.001) 
//#define L9bvb  (0.000666) // for biases
//#define G4asu  (0.00060) 
//#define L9asu  (0.00040) // for biases
//#define G4sdsu (0.003) 
//#define L9sdsu (0.002) // for biases

// 19 
// 0.635710        0.900644        176.150000
//#define G4abu  (0.001) 
//#define L9abu  (0.000666) // for biases
//#define G4sdbu (0.03) 
//#define L9sdbu (0.01) // for biases
//#define G4bvb  (0.0006) 
//#define L9bvb  (0.0004) // for biases
//#define G4asu  (0.00060) 
//#define L9asu  (0.00040) // for biases
//#define G4sdsu (0.003) 
//#define L9sdsu (0.002) // for biases

// 20 
// 0.641985        0.904331        177.560000
//#define G4abu  (0.0006) 
//#define L9abu  (0.0004) // for biases
//#define G4sdbu (0.05) 
//#define L9sdbu (0.03333) // for biases
//#define G4bvb  (0.0006) 
//#define L9bvb  (0.0004) // for biases
//#define G4asu  (0.00060) 
//#define L9asu  (0.00040) // for biases
//#define G4sdsu (0.003) 
//#define L9sdsu (0.002) // for biases

// 21 Good
// 0.643299        0.897417        174.360000
//#define G4abu  (0.001) 
//#define L9abu  (0.000666) // for biases
//#define G4sdbu (0.009) 
//#define L9sdbu (0.006) // for biases
//#define G4bvb  (0.002) 
//#define L9bvb  (0.00133) // for biases
//#define G4asu  (0.00060) 
//#define L9asu  (0.00040) // for biases
//#define G4sdsu (0.003) 
//#define L9sdsu (0.002) // for biases

// 22
// 0.646169        0.897463        174.460000
//#define G4abu  (0.002) 
//#define L9abu  (0.00133) // for biases
//#define G4sdbu (0.007) 
//#define L9sdbu (0.0049) // for biases
//#define G4bvb  (0.002) 
//#define L9bvb  (0.00133) // for biases
//#define G4asu  (0.00060) 
//#define L9asu  (0.00040) // for biases
//#define G4sdsu (0.003) 
//#define L9sdsu (0.002) // for biases

// 23
// 0.645978        0.897499        174.500000
//#define G4abu  (0.001) 
//#define L9abu  (0.015) // for biases
//#define G4sdbu (0.009) 
//#define L9sdbu (0.006) // for biases
//#define G4bvb  (0.002) 
//#define L9bvb  (0.00133) // for biases
//#define G4asu  (0.00060) 
//#define L9asu  (0.00040) // for biases
//#define G4sdsu (0.003) 
//#define L9sdsu (0.002) // for biases

// 24
// 0.640968        0.897194        174.960000
// RMSE Train 0.640968 (37.0%) Probe 0.897194 (7.2%) Both 0.645263 (36.3%)
#define G4abu  (0.001) 
#define L9abu  (0.000666) // for biases
#define G4sdbu (0.007) 
#define L9sdbu (0.0049) // for biases
#define G4bvb  (0.002) 
#define L9bvb  (0.00133) // for biases
#define G4asu  (0.00060) 
#define L9asu  (0.00040) // for biases
#define G4sdsu (0.003) 
#define L9sdsu (0.002) // for biases
//#define G1 (0.0045) 
//#define G2 (0.0045) 
//#define L6 (0.0034) // for biases
//#define L7 (0.010) 

// 25 Worse
// 0.640913        0.897226        169.820000
// RMSE Train 0.640913 (37.0%) Probe 0.897226 (7.2%) Both 0.645210 (36.3%)
//#define G1 (0.0038) 
//#define G2 (0.0045) 
//#define L6 (0.0029) // for biases
//#define L7 (0.010) 

// 26 Worse
// 0.632467        0.897664        170.180000
// RMSE Train 0.632467 (38.8%) Probe 0.897664 (7.1%) Both 0.636947 (38.0%)
//#define G1 (0.0045) 
//#define G2 (0.0038) 
//#define L6 (0.0034) // for biases
//#define L7 (0.008) 

// Baseline
//#define G1 (0.0045) 
//#define G2 (0.0045) 
//#define L6 (0.0034) // for biases
//#define L7 (0.010) 

// 27 Worse
// RMSE Train 0.642802 (36.6%) Probe 0.897224 (7.2%) Both 0.647060 (35.9%)
//#define G2sY (0.0035) 
//#define L7sY (0.0075) 

// 28 Worse
// RMSE Train 0.639924 (37.2%) Probe 0.897209 (7.2%) Both 0.644241 (36.5%)
//#define G2sY (0.0055) 
//#define L7sY (0.012) 

// 29 Worse
// RMSE Train 0.630411 (39.3%) Probe 0.897842 (7.1%) Both 0.634938 (38.5%)
//#define G2sY (0.0045) 
//#define L7sY (0.010) 
//#define G1 (0.0045) 
//#define L6 (0.0034) // for biases
//#define G2 (0.0035) 
//#define L7 (0.0075) 

// 30
//#define G2sY (0.0045) 
//#define L7sY (0.010) 
//#define G1 (0.0045) 
//#define L6 (0.0034) // for biases
//#define G2 (0.0055) 
//#define L7 (0.0110) 

// 31
//#define G2sY (0.0045) 
//#define L7sY (0.010) 
//#define G1 (0.0035) 
//#define L6 (0.0028) // for biases
//#define G2 (0.0045) 
//#define L7 (0.01) 

// 32
#define G2sY (0.0045) 
#define L7sY (0.010) 
#define G1 (0.0055) 
#define L6 (0.0042) // for biases
#define G2 (0.0045) 
#define L7 (0.01) 



//#define LbU (0.015000)
//#define LbV (0.015000)
////#define LabU (0.022000)
//#define LsU (0.015000)
//#define LsV (0.015000)
//#define LsY (0.015000)
////#define LasU (0.022000)
//#define LabU (0.015887)
//#define LasU (0.015887)

// NREG - bU: 0.109323 bV: 0.220189, sU: 0.054395, sV: 0.035104, sY: 0.035953, abU: 0.097887, asU: 0.097887
// NREG - bU: 0.108849 bV: 0.203866, sU: 0.054361, sV: 0.002488, sY: 0.038758, abU: 0.096266, asU: -0.022972
// NREG - bU: 0.107506 bV: 0.201122, sU: 0.064204, sV: 0.002017, sY: 0.066044, abU: 0.089061, asU: -0.021252
// NREG - bU: 0.107506 bV: 0.201122, sU: 0.064204, sV: 0.002017, sY: 0.066044, abU: 0.089061, asU: -0.021252
// NREG - bU: 0.107506 bV: 0.201122, sU: 0.064204, sV: 0.002017, sY: 0.066044, abU: 0.089061, asU: -0.036915
// NREG - bU: 0.100789 bV: 0.133174, sU: 0.053893, sV: 0.002462, sY: 0.038417, abU: 0.081736
// REG - bU: 0.136004 bV: 0.150703, sU: 0.074469, sV: 0.001840, sY: 0.045442, abU: 0.081450

//#define DTIME(t) (sqrt((t)&0xfffffff0))

#define TOTAL_BINS          30
#define TOTAL_DAYS_RANGE    2243
#define DAYS_PER_BIN        75    


double       avgdate[NUSERS];
double       avgdevu[NUSERS];
int          ucnt[NUSERS];
float        maxDEVuHat[NUSERS];
//unsigned int sdbin[NENTRIES];
unsigned int *sdbin;
unsigned int sdcount;
float       *DEVuHat;
float       *sdbU;
float       *sdsU;
int          minday=100000000;
int          maxday=-1000000;


int sign(double v) {
	if ( v > 0.0 ) 
		return 1;
	else if ( v < 0.0 ) 
		return -1;
	return 0;
}

double devu(int day, int u) {
    double DEVu = sign(day - avgdate[u]) * powf(abs(day - avgdate[u]), 0.4);
	return DEVu;
}

double devuHat(int day, int u) {
	//double DEVu = devu(day, u);
    double DEVu = sign(day - avgdate[u]) * powf(abs(day - avgdate[u]), 0.4);
    if ( maxDEVuHat[u] == 0.0 )
		return 0.0;
    return DEVu / maxDEVuHat[u];
}

int dbin(int day) {
	//if (dates[i] > 0) {
		//bias_index[i] = (int) ceil(dates[i] / (float) DAYS_PER_BIN) - 1;
	//} else {
		//bias_index[i] = 0;
	//}
	int nday = day - minday;

	int drange = nday / DAYS_PER_BIN;

	return drange;
}


FILE *fpV=NULL, *fpU=NULL;
int moviecount[NMOVIES];
void score_setup()
{
	printf("STARTED SETUP\n");
	fflush(stdout);

	int i,u;
    //weight_time_setup();
	if(load_model) {
		fpV=fopen(fnameV,"rb");
		fpU=fopen(fnameU,"rb");
		if(fpV || fpU) {
			lg("Loading %s and %s\n",fnameV,fnameU);
			if(!fpV || !fpU)
				error("Cant open both files");
		}
	}

	printf("HERE 0\n");
	fflush(stdout);

    int day0[NMOVIES];
    ZERO(day0);

	printf("HERE 0b\n");
	fflush(stdout);

    // It is OK to look on all data for day0 because it is always known
    for(i=0;i<NENTRIES;i++) {
        int m=userent[i]&USER_MOVIEMASK;
        int day=userent[i]>>(USER_LMOVIEMASK+3);
        if(!day0[m] || day0[m]>day) day0[m]=day;
    }
	printf("HERE 0c\n");
	fflush(stdout);
    printf("size = %d\n", (unsigned int)NENTRIES*(unsigned int)sizeof(float));
	fflush(stdout);
    DEVuHat = (float *) malloc((unsigned int)NENTRIES*(unsigned int)sizeof(float));
	printf("HERE 0d DEVuHat: %d \n", DEVuHat);
	fflush(stdout);
	sdbin   = (unsigned int *) malloc(NENTRIES *sizeof(unsigned int));
	printf("HERE 0e\n");
	fflush(stdout);
	for (i=0; i < NENTRIES; i++)
		DEVuHat[i] = 0.0;
	//memset(DEVuHat,0,(unsigned int)NENTRIES*(unsigned int)sizeof(float));
	printf("HERE 0f\n");
	fflush(stdout);
	memset(sdbin,0,NENTRIES*sizeof(unsigned int));

	printf("HERE 1\n");
	fflush(stdout);


	// Allocate sU and alphasU on the heap
	//
	// double sU[NUSERS][NFEATURES];
    // double alphasU[NUSERS][NFEATURES];
	//
	sU = (double **)malloc(NUSERS * sizeof(double *));
	sU[0] = (double *)malloc(NUSERS * NFEATURES * sizeof(double));
	for(i = 1; i < NUSERS; i++)
		sU[i] = sU[0] + i * NFEATURES;
	alphasU = (double **)malloc(NUSERS * sizeof(double *));
	alphasU[0] = (double *)malloc(NUSERS * NFEATURES * sizeof(double));
	for(i = 1; i < NUSERS; i++)
		alphasU[i] = alphasU[0] + i * NFEATURES;

	printf("HERE 2\n");
	fflush(stdout);

	// double sV[NMOVIES][NFEATURES];
	// double sY[NMOVIES][NFEATURES];
	sV = (double **)malloc(NMOVIES * sizeof(double *));
	sV[0] = (double *)malloc(NMOVIES * NFEATURES * sizeof(double));
	for(i = 1; i < NMOVIES; i++)
		sV[i] = sV[0] + i * NFEATURES;
	sY = (double **)malloc(NMOVIES * sizeof(double *));
	sY[0] = (double *)malloc(NMOVIES * NFEATURES * sizeof(double));
	for(i = 1; i < NMOVIES; i++)
		sY[i] = sY[0] + i * NFEATURES;

	printf("HERE 3\n");
	fflush(stdout);


	int tcount[100000];
	ZERO(tcount);
	ZERO(avgdate);
	ZERO(avgdevu);
	ZERO(ucnt);
	int j;
    for(u=0;u<NUSERS;u++) {
        int base=useridx[u][0];
        int d012=UNALL(u);
        int d0=UNTRAIN(u);
        // compute explanatory variable
        for(j=0;j<d012;j++) {
            int m=userent[base+j]&USER_MOVIEMASK;
            int day=userent[base+j]>>(USER_LMOVIEMASK+3);
			if ( day < minday ) 
				minday = day;
			if ( day > maxday ) 
				maxday = day;
            //usertime[j]=DTIME(day-day0[m]);
			if ( day < 0 )
				;//toosmall++;
			else if ( day < 100000-1 )
			    tcount[day]++;
			else 
				;//toobig++;
			ucnt[u]++;
			avgdate[u] += day;
        }
    }
	printf("minday: %d, maxday: %d\n", minday, maxday);
	fflush(stdout);
    for(u=0;u<NUSERS;u++) {
        // 1) Find the average date of rating for every customer. In this step I include the probe dates also in the calculation of the average.
		avgdate[u] /= ucnt[u];
	    //printf("U: %d, avgdate: %f, ucnt: %d\n", u, avgdate[u], ucnt[u]);
		//fflush(stdout);
	}
    for(u=0;u<NUSERS;u++) {
        int base=useridx[u][0];
        int d012=UNALL(u);
        int d0=UNTRAIN(u);
        int j;
        for(j=0;j<d012;j++) {
            int m=userent[base+j]&USER_MOVIEMASK;
            int day=userent[base+j]>>(USER_LMOVIEMASK+3);
  			//2) For every rating [i] in the data set (including probe) I calculate DEVu[i]:
      		//   DEVu[i] = sign(t[i] - t_mean_for_customer) * powf(abs(t[i] - t_mean_for_customer), 0.4);
			double DEVu = sign(day - avgdate[u]) * powf(abs(day - avgdate[u]), 0.4);
			avgdevu[u] += DEVu;
	        //printf("U: %d, M: %d, day: %d, uavg: %f, DEVu: %f\n", u, m, day, avgdate[u], DEVu);
		    //fflush(stdout);

        }
    }
    for(u=0;u<NUSERS;u++) {
        //3) Find the average DEVu[i] for every customer. His/hers probe DEVu[i] values are also included.
		avgdevu[u] /= ucnt[u];
	    //printf("U: %d, avgdevu: %f, avgdate: %f, ucnt: %d\n", u, avgdevu[u], avgdate[u], ucnt[u]);
		//fflush(stdout);
	}
	ZERO(maxDEVuHat);
    for(u=0;u<NUSERS;u++) {
        int base=useridx[u][0];
        int d012=UNALL(u);
        int d0=UNTRAIN(u);
        int j;
        for(j=0;j<d012;j++) {
            int m=userent[base+j]&USER_MOVIEMASK;
            int day=userent[base+j]>>(USER_LMOVIEMASK+3);

  			//  2) For every rating [i] in the data set (including probe) I calculate DEVu[i]:
      		//     DEVu[i] = sign(t[i] - t_mean_for_customer) * powf(abs(t[i] - t_mean_for_customer), 0.4);
			double DEVu = sign(day - avgdate[u]) * powf(abs(day - avgdate[u]), 0.4);

            //  4) Subtract every customer's average DEVu_avg value from every time deviation:
            //     DEVu_hat[i] = DEVu[i] - DEVu_avg_for_customer;
            double DEVuHat = DEVu - avgdevu[u];

	        //printf("U: %d, M: %d, ndevu: %f, day: %d, uavg: %f, DEVu: %f\n", u, m, DEVuHat, day, avgdate[u], DEVu);
		    //fflush(stdout);

	    	// Get the max absolute value of a user's devu_hat values...maxDevu_hat...
			double tDEVu = fabs(DEVuHat);
			if ( tDEVu > maxDEVuHat[u] )
				maxDEVuHat[u] = tDEVu;
        }
    }
	
	// Compute and store DEVuHats and create single day bin numbering per user
	int daysBinValue[maxday+1];
	sdcount=1;
    for(u=0;u<NUSERS;u++) {
        int base=useridx[u][0];
        int d012=UNALL(u);
        int d0=UNTRAIN(u);
        int j;
		ZERO(daysBinValue);
        for(j=0;j<d012;j++) {
            int m=userent[base+j]&USER_MOVIEMASK;
            int day=userent[base+j]>>(USER_LMOVIEMASK+3);

            DEVuHat[base+j] = devuHat(day,u);

			if ( daysBinValue[day] == 0 ) {
				//sdbin[base+j] = base+j;
			    //daysBinValue[day] = base+j;
				sdbin[base+j] = sdcount;
			    daysBinValue[day] = sdcount;
				if ( daysBinValue[day] > NENTRIES ) {
					printf("Days bin v: %d\n", daysBinValue[day]);
					fflush(stdout);
				}
				sdcount++;
			} else {
				if ( daysBinValue[day] > NENTRIES ) {
					printf("Days bin v: %d\n", daysBinValue[day]);
					fflush(stdout);
				}
				sdbin[base+j] = daysBinValue[day];
			}
        }
    }
	printf("sdcount=%d\n", sdcount);
	fflush(stdout);
    sdbU    = (float *) malloc(sdcount*sizeof(float));
    sdsU    = (float *) malloc(((unsigned int)sdcount)*((unsigned int)NFEATURES)*sizeof(float));
	memset(sdbU,0,sdcount*sizeof(float));
	memset(sdsU,0,((unsigned int)sdcount)*((unsigned int)NFEATURES)*sizeof(float));

	//for (i=minday; i < maxday; i++ ) {
	    //printf("day: %d, count: %d\n", i, tcount[i]);
		//fflush(stdout);
	//}

	printf("DONE SETUP\n");
	fflush(stdout);

}


void removeUV()
{
	int u,f;
	for(u=0;u<NUSERS;u++) {
		int base0=useridx[u][0];
		int d012=UNALL(u);
		int i;

		int dall=UNALL(u);
		double NuS = 1.0/sqrt(dall);
		double lNuSY[NFEATURES];
		double sumY[NFEATURES];
		ZERO(sumY);
		ZERO(lNuSY);
		int j;
		for(j=0;j<dall;j++) {
			int mm=userent[base0+j]&USER_MOVIEMASK;
			for(f=0;f<NFEATURES;f++)
				sumY[f]+=sY[mm][f];
		}
		int d0=UNTRAIN(u);
		for(f=0;f<NFEATURES;f++) 
			lNuSY[f] = NuS * sumY[f]; 


        //double bUu=bU[u];
		//for(i=0; i<d012;i++) {
			//int m=userent[base0+i]&USER_MOVIEMASK;
			//err[base0+i]-=(bU[u] + bV[m]);
			//for (f=0; f<NFEATURES; f++)
			    //err[base0+i]-=((sU[u][f] + lNuSY[f]) * sV[m][f]);
		//}

		// For all rated movies
		for(i=0;i<d012;i++) {
			int entloc = base0+i;
			unsigned int sdloc = sdbin[entloc];
			int m=userent[entloc]&USER_MOVIEMASK;
			int day=userent[entloc]>>(USER_LMOVIEMASK+3);
			double devuhat = DEVuHat[entloc];

			// Figure out the current error
			err[entloc] -= (bU[u] + bV[m] + bVbin[m][dbin(day)] + sdbU[sdloc] + alphabU[u] * devuhat);
			for (f=0; f<NFEATURES; f++)
				err[entloc] -= (( sU[u][f] + sdsU[sdloc+f*sdcount] +  alphasU[u][f] * devuhat + lNuSY[f]) * sV[m][f]);
		}
	}
}

int score_train(int loop)
{
	if (loop == 0)
		return doAllFeatures();
	
	return 1;
}

int doAllFeatures()
{
	/* Initial biases */
	{
		int u,m;
		//memset(sdbU,0,NENTRIES*sizeof(float));
		//memset(sdsU,0,((unsigned int)NENTRIES)*((unsigned int)NFEATURES)*sizeof(float));
		ZERO(bVbin);
		ZERO(alphabU);
		
		for(u=0;u<NUSERS;u++) {
			bU[u]=0.0;
		}
		for(m=0;m<NMOVIES;m++) {
			bV[m]=0.0;
		}
	}
	
	
	/* Initial estimation for current feature */
	{
		int u,m,f;
		
		double uvInit = sqrt(GLOBAL_MEAN/NFEATURES);
		for(u=0;u<NUSERS;u++) {
			for(f=0;f<NFEATURES;f++) {
			    sU[u][f]      = uvInit * (rand()%14000 + 2000) * 0.000001235f;
			    alphasU[u][f] = 0.0;
			}
		}
		for(m=0;m<NMOVIES;m++) {
			for(f=0;f<NFEATURES;f++) {
			    sV[m][f]= uvInit * (rand()%14000 + 2000) * -0.000001235f;
			    sY[m][f]=0.0;
			}
		}
	}
	
	/* Optimize current feature */
	double nrmse=2., last_rmse=10.;
	double prmse = 0, last_prmse=0;
	double thr=sqrt(1.-E);
	int loopcount=0;
	double Gamma1 = G1;
	double Gamma2 = G2;
	double Gamma2sY = G2sY;
	//double Gamma4 = G4;
	double Gamma4abu = G4abu;
	double Gamma4sdbu = G4sdbu;
	double Gamma4bvb = G4bvb;
	double Gamma4sdsu = G4sdsu;
	double Gamma4asu = G4asu;
	while( ((nrmse < (last_rmse-E) && prmse<last_prmse) || loopcount < 15) && loopcount < 40  )  {
		last_rmse=nrmse;
		last_prmse=prmse;
		clock_t t0=clock();
		loopcount++;

		double aErrAvg=0;
		double aEDAvg=0;
		double astepSuAvg=0;
		double astepSvAvg=0;
		double astepSyAvg=0;
		double aasU=0, aabU=0, abU=0, abV=0, asU=0, asV=0, asY=0;
		long n1=0, n2=0, n3=0;
		int elcnt=0;

		int u,m, f;
		for(u=0;u<NUSERS;u++) {

			// Calculate sumY and NuSY for each factor
			double sumY[NFEATURES];
			ZERO(sumY);
			double lNuSY[NFEATURES];
			ZERO(lNuSY);
			int base0=useridx[u][0];
			int d0=UNTRAIN(u);
			int j;
			int f;
			int dall=UNALL(u);
			double NuS = 1.0/sqrt(dall);
			for(j=0;j<dall;j++) {
				int mm=userent[base0+j]&USER_MOVIEMASK;
				for(f=0;f<NFEATURES;f++)
					sumY[f]+=sY[mm][f];
			}
			for(f=0;f<NFEATURES;f++) {
				lNuSY[f] = NuS * sumY[f]; 
			}

			double ycontrib[NFEATURES];
			ZERO(ycontrib);

			// For all rated movies
			for(j=0;j<d0;j++) {
				int entloc = base0+j;
				unsigned int sdloc = sdbin[entloc];
				int m=userent[entloc]&USER_MOVIEMASK;
                int day=userent[entloc]>>(USER_LMOVIEMASK+3);
				double devuhat = DEVuHat[entloc];

				// Figure out the current error
				double ee=err[entloc];
				double e2 = ee;

				//e2 -= (bU[u] + bV[m] + bVbin[m][dbin(day)] + sdbU[sdloc] + alphabU[u] * devuhat);
				e2 -= bU[u];
				e2 -= bV[m];
				e2 -= bVbin[m][dbin(day)];
				e2 -= sdbU[sdloc];
				e2 -= alphabU[u] * devuhat;
				for (f=0; f<NFEATURES; f++)
					e2 -= (( sU[u][f] + sdsU[sdloc+f*sdcount] +  alphasU[u][f] * devuhat + lNuSY[f]) * sV[m][f]);

				int r=(userent[entloc]>>USER_LMOVIEMASK)&7;
				r++;
				double rui = r - e2;
				if ( rui > 5.00 )
					e2 += (rui-5.0);
				else if (rui < 1.0)
					e2 -= (1.0 - rui);


				// Train the biases
				double bUu = bU[u];
				double bVm = bV[m];
				bU[u] += Gamma1 * (e2 - bUu * L6);
				bV[m] += Gamma1 * (e2 - bVm * L6);
				alphabU[u]          += Gamma4abu * (e2 * devuhat - (L9abu) * alphabU[u]);	
				sdbU[sdloc]         += Gamma4sdbu * (e2 - (L9sdbu) * sdbU[sdloc]);	
				bVbin[m][dbin(day)] += Gamma4bvb * (e2 - bVbin[m][dbin(day)] * (L9bvb));
//aErrAvg+=fabs(e2);
//aEDAvg+=fabs(e2*devuhat);
//abU += fabs(bU[u]);
//abV += fabs(bV[m]);
//aabU += fabs(alphabU[u]);
//n1++;

				// update U V and slope component of Y
				double yfactor = NuS;
				for (f=0; f<NFEATURES; f++) {
					double sUu = sU[u][f];
					double sVm = sV[m][f];

					sU[u][f] += ((Gamma2) * ((e2 * sVm) - L7 * sUu));
					sV[m][f] += ((Gamma2) * ((e2 * (sUu + sdsU[sdloc+f*sdcount] + alphasU[u][f] * devuhat + lNuSY[f])) - L7 * sVm));
					alphasU[u][f]           += Gamma4asu * (e2 * devuhat * sVm - (L9asu) * alphasU[u][f]);	
			  	    sdsU[sdloc+f*sdcount]  += Gamma4sdsu * (e2 * sVm - (L9sdsu) * sdsU[sdloc+f*sdcount]);	
//asU += fabs(sU[u][f]);
//asV += fabs(sV[m][f]);
//aasU += fabs(alphasU[u][f]);
//astepSuAvg+=fabs(e2 * sV[m][f]);
//astepSvAvg+=fabs(e2 * (sU[u][f] + sdsU[sdloc+f*sdcount] + alphasU[u][f] * devuhat + lNuSY[f])); 
//n2++;
	
					ycontrib[f] += e2 * sVm * yfactor;
				}
if( elcnt++ == 5000 ) {
    printf("0 E: %f \t NE: %f\tNuSY: %f\tsV: %f\tsU: %f\tbU: %f\tbV: %f\tsY: %f\tU: %d\tM: %d\n", ee, e2, lNuSY[0], sV[m][0], sU[u][0], bU[u], bV[m], sY[m][0],u, m);
    printf("1 E: %f \t NE: %f\tNuSY: %f\tsV: %f\tsU: %f\tbU: %f\tbV: %f\tsY: %f\tU: %d\tM: %d\n", ee, e2, lNuSY[1], sV[m][1], sU[u][1], bU[u], bV[m], sY[m][1],u, m);
    printf("2 E: %f \t NE: %f\tNuSY: %f\tsV: %f\tsU: %f\tbU: %f\tbV: %f\tsY: %f\tU: %d\tM: %d\n", ee, e2, lNuSY[2], sV[m][2], sU[u][2], bU[u], bV[m], sY[m][2],u, m);
    printf("0 abU: %f \t sdbU: %f\tbvbin: %f\tasU: %f\tsdsU: %f\n", alphabU[u], sdbU[sdloc], bVbin[m][dbin(day)], alphasU[u][0], sdsU[sdloc+0*sdcount]);
    printf("1 abU: %f \t sdbU: %f\tbvbin: %f\tasU: %f\tsdsU: %f\n", alphabU[u], sdbU[sdloc], bVbin[m][dbin(day)], alphasU[u][1], sdsU[sdloc+1*sdcount]);
	fflush(stdout);
}
			}

			// Train Ys over all known movies for user
			for(j=0;j<dall;j++) {
				int m=userent[base0+j]&USER_MOVIEMASK;
				for (f=0; f<NFEATURES; f++) {
					double sYm = sY[m][f];
					sY[m][f] += Gamma2sY * (ycontrib[f] - L7sY * sYm);
//asY += fabs(sY[m][f]);
//astepSyAvg+=fabs(ycontrib[f]);
//n3++;
				}
			}
		}


        //aErrAvg/=n1;
        //aEDAvg/=n1;
        //astepSuAvg/=n2;
        //astepSvAvg/=n2;
        //astepSyAvg/=n3;
        //aasU/=n2,aabU/=n1, abU/=n1, abV/=n1, asU/=n2, asV/=n2, asY/=n2;
        //double bUREG = 1.9074 / 100.0 * aErrAvg / abU;
        //double bVREG = 1.9074 / 100.0 * aErrAvg / abV;
        //double abUREG= 1.9074 / 100.0 * aEDAvg / aabU;
        //double sUREG = 1.9074 / 100.0 * astepSuAvg / asU;
        //double sVREG = 1.9074 / 100.0 * astepSvAvg / asV;
        //double sYREG = 1.9074 / 100.0 * astepSyAvg / asY;
        //double asUREG= 1.9074 / 100.0 * aEDAvg / aasU;
        //printf("NREG - bU: %f bV: %f, sU: %f, sV: %f, sY: %f, abU: %f, asU: %f\n", bUREG, bVREG, sUREG, sVREG, sYREG, abUREG, asUREG);


		// Report rmse for main loop
		nrmse=0.;
		int ntrain=0;
		int k=2;
		int n=0;
		double s=0.;
		for(u=0;u<NUSERS;u++) {
			int base0=useridx[u][0];
			int d0=UNTRAIN(u);
			int j;

			// Setup the Ys again
			double sumY[NFEATURES];
			ZERO(sumY);
			double lNuSY[NFEATURES];
			ZERO(lNuSY);
			int dall=UNALL(u);
			double NuS = 1.0/sqrt(dall);
			for(j=0;j<dall;j++) {
				int mm=userent[base0+j]&USER_MOVIEMASK;
				for(f=0;f<NFEATURES;f++)
					sumY[f]+=sY[mm][f];
			}
			for(f=0;f<NFEATURES;f++) 
				lNuSY[f] = NuS * sumY[f]; 

			// For all rated movies
			for(j=0;j<d0;j++) {
				int entloc = base0+j;
				unsigned int sdloc = sdbin[entloc];
				int m=userent[entloc]&USER_MOVIEMASK;
                int day=userent[entloc]>>(USER_LMOVIEMASK+3);
				double devuhat = DEVuHat[entloc];

				// Figure out the current error
				double ee=err[entloc];
				double e2 = ee;
				e2 -= (bU[u] + bV[m] + bVbin[m][dbin(day)] + sdbU[sdloc] + alphabU[u] * devuhat);
				for (f=0; f<NFEATURES; f++)
					e2 -= (( sU[u][f] + sdsU[sdloc+f*sdcount] +  alphasU[u][f] * devuhat + lNuSY[f]) * sV[m][f]);

			//for(j=0;j<d0;j++) {
				//int m=userent[base0+j]&USER_MOVIEMASK;
				//double ee = err[base0+j];
				//double e2 = ee;
				//e2 -= (bU[u] + bV[m]);
				//for (f=0; f<NFEATURES; f++)
					//e2 -= ( (sU[u][f] + lNuSY[f]) * sV[m][f]);

				int r=(userent[base0+j]>>USER_LMOVIEMASK)&7;
				r++;
				double rui = r - e2;
				if ( rui > 5.00 )
					e2 += (rui-5.0);
				else if (rui < 1.0)
					e2 -= (1.0 - rui);


				nrmse+=e2*e2;
			}
			ntrain+=d0;

			// Sum up probe rmse
			int i;
			int base=useridx[u][0];
			for(i=1;i<k;i++) base+=useridx[u][i];
			int d=useridx[u][k];
			for(i=0; i<d;i++) {
				int entloc = base+i;
				unsigned int sdloc = sdbin[entloc];
				int m=userent[entloc]&USER_MOVIEMASK;
                int day=userent[entloc]>>(USER_LMOVIEMASK+3);
				double devuhat = DEVuHat[entloc];

				//double e=err[entloc];
				//e-=(bU[u] + bV[m]);
				//for (f=0; f<NFEATURES; f++)
					//e-=((sU[u][f] + lNuSY[f]) * sV[m][f]);

				double ee=err[entloc];
				double e = ee;
				e -= (bU[u] + bV[m] + bVbin[m][dbin(day)] + sdbU[sdloc] + alphabU[u] * devuhat);
				for (f=0; f<NFEATURES; f++)
					e -= (( sU[u][f] + sdsU[sdloc+f*sdcount] +  alphasU[u][f] * devuhat + lNuSY[f]) * sV[m][f]);

				int r=(userent[entloc]>>USER_LMOVIEMASK)&7;
				r++;
				double rui = r - e;
				if ( rui > 5.00 )
					e += (rui-5.0);
				else if (rui < 1.0)
					e -= (1.0 - rui);

				s+=e*e;
			}
			n+=d;
		}
		nrmse=sqrt(nrmse/ntrain);
		prmse = sqrt(s/n);
		
		lg("%f\t%f\t%f\n",nrmse,prmse,(clock()-t0)/(double)CLOCKS_PER_SEC);
		Gamma1 *= 0.90;
		Gamma2 *= 0.90;
		Gamma2sY *= 0.90;
		//Gamma4 *= 0.90;
		Gamma4abu *= 0.90;
		Gamma4sdbu *= 0.90;
		Gamma4bvb *= 0.90;
		Gamma4asu *= 0.90;
		Gamma4sdsu *= 0.90;
	}
	
	/* Perform a final iteration in which the errors are clipped and stored */
	removeUV();
	
	//if(save_model) {
		//dappend_bin(fnameV,sV,NMOVIES);
		//dappend_bin(fnameU,sU,NUSERS);
	//}
	
	return 1;
}
