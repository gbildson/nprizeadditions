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



#define NFEATURES (35)
#define GLOBAL_MEAN (3.6033)
double sU[NUSERS][NFEATURES];
double sV[NMOVIES][NFEATURES];
double sY[NMOVIES][NFEATURES];
double bU[NUSERS]; 
double bV[NMOVIES];
double bVbin[NMOVIES][30];
double alphabU[NUSERS];
double alphasU[NUSERS][NFEATURES];


//#define G1 (0.007) 
//#define G2 (0.007) 
//#define L6 (0.005) // for biases
//#define L7 (0.015) 
////#define G4 (0.001) 
////#define L9 (0.002) // for biases
//#define G4 (0.01) 
//#define L9 (0.006) // for biases

#define G1 (0.0111) 
#define G2 (0.0111) 
#define G4 (0.0111) 
#define L6 (0.005) // for biases
#define L7 (0.015) 
#define L9 (0.006) // for biases

//#define LbU (0.100789)
//#define LbV (0.133174)
//#define LsU (0.053893)
//#define LsV (0.053893)
////#define LsV (0.002462)
//#define LsY (0.038417)
//#define LabU (0.081736)
//#define LasU (0.036915)

//#define LbU (0.108849)
//#define LbV (0.203866)
//#define LsU (0.054361)
//#define LsV (0.054361)
//#define LsY (0.038758)
//#define LabU (0.096266)
//#define LasU (0.022972)

#define LbU (0.109323)
#define LbV (0.220189)
#define LabU (0.097887)
//#define LsU (0.054395)
//#define LsV (0.035104)
//#define LsY (0.035953)
#define LsU (0.054395)
#define LsV (0.054395)
#define LsY (0.054395)
#define LasU (0.097887)

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
unsigned int sdbin[NENTRIES];
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

    int day0[NMOVIES];
    ZERO(day0);
    // It is OK to look on all data for day0 because it is always known
    for(i=0;i<NENTRIES;i++) {
        int m=userent[i]&USER_MOVIEMASK;
        int day=userent[i]>>(USER_LMOVIEMASK+3);
        if(!day0[m] || day0[m]>day) day0[m]=day;
    }
    DEVuHat = (float *) malloc(NENTRIES*sizeof(float));
    sdbU    = (float *) malloc(NENTRIES*sizeof(float));
    sdsU    = (float *) malloc(((unsigned int)NENTRIES)*((unsigned int)NFEATURES)*sizeof(float));
	memset(DEVuHat,0,NENTRIES*sizeof(float));
	memset(sdbU,0,NENTRIES*sizeof(float));
	memset(sdsU,0,((unsigned int)NENTRIES)*((unsigned int)NFEATURES)*sizeof(float));
	ZERO(sdbin);

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
    for(u=0;u<NUSERS;u++) {
        int base=useridx[u][0];
        int d012=UNALL(u);
        int d0=UNTRAIN(u);
        int j;
		ZERO(daysBinValue);
		int dcount=0;
        for(j=0;j<d012;j++) {
            int m=userent[base+j]&USER_MOVIEMASK;
            int day=userent[base+j]>>(USER_LMOVIEMASK+3);

            DEVuHat[base+j] = devuHat(day,u);

			if ( daysBinValue[day] == 0 ) {
				sdbin[base+j] = base+j;
			    daysBinValue[day] = base+j;
				if ( daysBinValue[day] > NENTRIES ) {
					printf("Days bin v: %d\n", daysBinValue[day]);
					fflush(stdout);
				}
				dcount++;
			} else {
				if ( daysBinValue[day] > NENTRIES ) {
					printf("Days bin v: %d\n", daysBinValue[day]);
					fflush(stdout);
				}
				sdbin[base+j] = daysBinValue[day];
			}
        }
    }

	//for (i=minday; i < maxday; i++ ) {
	    //printf("day: %d, count: %d\n", i, tcount[i]);
		//fflush(stdout);
	//}

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
				err[entloc] -= (( sU[u][f] + sdsU[sdloc+f*NENTRIES] +  alphasU[u][f] * devuhat + lNuSY[f]) * sV[m][f]);
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
		ZERO(alphasU);
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
	double Gamma4 = G4;
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
					e2 -= (( sU[u][f] + sdsU[sdloc+f*NENTRIES] +  alphasU[u][f] * devuhat + lNuSY[f]) * sV[m][f]);

				int r=(userent[entloc]>>USER_LMOVIEMASK)&7;
				r++;
				double rui = r - e2;
				if ( rui > 5.00 )
					e2 += (rui-5.0);
				else if (rui < 1.0)
					e2 -= (1.0 - rui);


				// Train the biases
				double bUu  = bU[u];
				double bVm  = bV[m];
				double abUu = alphabU[u];
				double sdbUu= sdbU[sdloc];	
				bU[u] += Gamma1 * (e2 - bUu * LbU);
				bV[m] += Gamma1 * (e2 - bVm * LbV);
				alphabU[u]          += Gamma4 * (e2 * devuhat - (LabU) * abUu);	
				sdbU[sdloc]         += Gamma4 / TOTAL_DAYS_RANGE * (e2 - (LbU) * sdbUu);	
				bVbin[m][dbin(day)] += Gamma4 / 30.0 * (e2 - bVbin[m][dbin(day)] * (LbV));
aErrAvg+=fabs(e2);
aEDAvg+=fabs(e2*devuhat);
abU += fabs(bUu);
abV += fabs(bVm);
aabU += fabs(alphabU[u]);
n1++;

				// update U V and slope component of Y
				double yfactor = NuS;
				for (f=0; f<NFEATURES; f++) {
					double sUu = sU[u][f];
					double sVm = sV[m][f];
					double sdsUu = sdsU[sdloc+f*NENTRIES];
					double asUu  = alphasU[u][f];

					sU[u][f] += ((Gamma2) * ((e2 * sVm) - LsU * sUu));
					sV[m][f] += ((Gamma2) * ((e2 * (sUu + sdsUu + asUu * devuhat + lNuSY[f])) - LsV * sVm));
					alphasU[u][f] += Gamma4 * (e2 * devuhat - (LabU) * asUu);	
			  	    sdsU[sdloc+f*NENTRIES]  += Gamma4 /TOTAL_DAYS_RANGE  * (e2 * sVm - (LsU) * sdsUu);	
asU += fabs(sUu);
asV += fabs(sVm);
aasU += fabs(asUu);
astepSuAvg+=fabs(e2 * sVm);
astepSvAvg+=fabs(e2 * (sUu + sdsUu + asUu * devuhat + lNuSY[f])); 
n2++;
	
					ycontrib[f] += e2 * sVm * yfactor;
				}
			}

			// Train Ys over all known movies for user
			for(j=0;j<dall;j++) {
				int m=userent[base0+j]&USER_MOVIEMASK;
				for (f=0; f<NFEATURES; f++) {
					double sYm = sY[m][f];
					sY[m][f] += Gamma2 * (ycontrib[f] - LsY * sYm);
asY += fabs(sYm);
astepSyAvg+=fabs(ycontrib[f]);
n3++;
				}
			}
		}


        aErrAvg/=n1;
        aEDAvg/=n1;
        astepSuAvg/=n2;
        astepSvAvg/=n2;
        astepSyAvg/=n3;
        aasU/=n2,aabU/=n1, abU/=n1, abV/=n1, asU/=n2, asV/=n2, asY/=n2;
        double bUREG = 1.9074 / 100.0 * aErrAvg / abU;
        double bVREG = 1.9074 / 100.0 * aErrAvg / abV;
        double abUREG= 1.9074 / 100.0 * aEDAvg / aabU;
        double sUREG = 1.9074 / 100.0 * astepSuAvg / asU;
        double sVREG = 1.9074 / 100.0 * astepSvAvg / asV;
        double sYREG = 1.9074 / 100.0 * astepSyAvg / asY;
        double asUREG= 1.9074 / 100.0 * aEDAvg / aasU;
        printf("NREG - bU: %f bV: %f, sU: %f, sV: %f, sY: %f, abU: %f, asU: %f\n", bUREG, bVREG, sUREG, sVREG, sYREG, abUREG, asUREG);


		// Report rmse for main loop
		nrmse=0.;
		int ntrain=0;
		int elcnt=0;
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
					e2 -= (( sU[u][f] + sdsU[sdloc+f*NENTRIES] +  alphasU[u][f] * devuhat + lNuSY[f]) * sV[m][f]);

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

if( elcnt++ == 5000 ) {
    printf("0 E: %f \t NE: %f\tNuSY: %f\tsV: %f\tsU: %f\tbU: %f\tbV: %f\tsY: %f\tU: %d\tM: %d\n", ee, e2, lNuSY[0], sV[m][0], sU[u][0], bU[u], bV[m], sY[m][0],u, m);
    printf("1 E: %f \t NE: %f\tNuSY: %f\tsV: %f\tsU: %f\tbU: %f\tbV: %f\tsY: %f\tU: %d\tM: %d\n", ee, e2, lNuSY[1], sV[m][1], sU[u][1], bU[u], bV[m], sY[m][1],u, m);
    printf("2 E: %f \t NE: %f\tNuSY: %f\tsV: %f\tsU: %f\tbU: %f\tbV: %f\tsY: %f\tU: %d\tM: %d\n", ee, e2, lNuSY[2], sV[m][2], sU[u][2], bU[u], bV[m], sY[m][2],u, m);
    printf("3 E: %f \t NE: %f\tNuSY: %f\tsV: %f\tsU: %f\tbU: %f\tbV: %f\tsY: %f\tU: %d\tM: %d\n", ee, e2, lNuSY[3], sV[m][3], sU[u][3], bU[u], bV[m], sY[m][3],u, m);
	fflush(stdout);
}

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
					e -= (( sU[u][f] + sdsU[sdloc+f*NENTRIES] +  alphasU[u][f] * devuhat + lNuSY[f]) * sV[m][f]);

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
		Gamma4 *= 0.90;
	}
	
	/* Perform a final iteration in which the errors are clipped and stored */
	removeUV();
	
	//if(save_model) {
		//dappend_bin(fnameV,sV,NMOVIES);
		//dappend_bin(fnameU,sU,NUSERS);
	//}
	
	return 1;
}
