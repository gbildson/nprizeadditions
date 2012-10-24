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



#define NFEATURES (50)
#define GLOBAL_MEAN (3.6033)
double sU[NUSERS][NFEATURES];
double sV[NMOVIES][NFEATURES];
double sY[NMOVIES][NFEATURES];
double bU[NUSERS]; 
double bV[NMOVIES];


//#define G1 (0.007) 
//#define G2 (0.007) 
//#define L6 (0.005) // for biases
//#define L7 (0.015) 
#define G1 (0.0111) 
#define G2 (0.0111) 
#define L6 (0.045) // for biases
#define L7 (0.045) 
#define LbU (0.090050)
#define LbV (0.141838)
//#define LsU (0.052207)
//#define LsV (0.002846)
#define LsU (0.0275265)
#define LsV (0.0275265)
#define LsY (0.037649)
//bU: 0.090050 bV: 0.141838, sU: 0.052207, sV: 0.002846, sY: 0.037649


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


        double bUu=bU[u];
		for(i=0; i<d012;i++) {
			int m=userent[base0+i]&USER_MOVIEMASK;
			err[base0+i]-=(bU[u] + bV[m]);
			for (f=0; f<NFEATURES; f++)
			    err[base0+i]-=((sU[u][f] + lNuSY[f]) * sV[m][f]);
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
			    sU[u][f]= uvInit * (rand()%14000 + 2000) * 0.000001235f;
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
	while( ((nrmse < (last_rmse-E) && prmse<last_prmse) || loopcount < 15) && loopcount < 40  )  {
		last_rmse=nrmse;
		last_prmse=prmse;
		clock_t t0=clock();
		loopcount++;

		double aErrAvg=0;
		double astepSuAvg=0;
		double astepSvAvg=0;
		double astepSyAvg=0;
		double abU=0, abV=0, asU=0, asV=0, asY=0;
		int n1=0, n2=0, n3=0;
		

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
				int m=userent[base0+j]&USER_MOVIEMASK;

				// Figure out the current error
				double ee=err[base0+j];
				double e2 = ee;
				e2 -= (bU[u] + bV[m]);
				for (f=0; f<NFEATURES; f++)
					e2 -= ((sU[u][f]+lNuSY[f])*sV[m][f]);
				//int r=(userent[base0+j]>>USER_LMOVIEMASK)&7;
				//r++;
				//double rui = r - e2;
				//if ( rui > 5.00 )
					//e2 += (rui-5.0);
				//else if (rui < 1.0)
					//e2 -= (1.0 - rui);

				// Train the biases
				double bUu = bU[u];
				double bVm = bV[m];
				//bU[u] += Gamma1 * (e2 - bUu * L6);
				//bV[m] += Gamma1 * (e2 - bVm * L6);
				bU[u] += Gamma1 * (e2 - bUu * LbU);
				bV[m] += Gamma1 * (e2 - bVm * LbV);

		aErrAvg+=fabs(e2);
		abU += fabs(bU[u]);
		abV += fabs(bV[m]);
		n1++;

				// update U V and slope component of Y
				double yfactor = NuS;
				for (f=0; f<NFEATURES; f++) {
					double sUu = sU[u][f];
					double sVm = sV[m][f];

					//sU[u][f] += ((Gamma2) * ((e2 * sVm) - L7 * sUu));
					//sV[m][f] += ((Gamma2) * ((e2 * (sUu + lNuSY[f])) - L7 * sVm));
					sU[u][f] += ((Gamma2) * ((e2 * sVm) - LsU * sUu));
					sV[m][f] += ((Gamma2) * ((e2 * (sUu + lNuSY[f])) - LsV * sVm));
		asU += fabs(sU[u][f]);
		asV += fabs(sV[m][f]);
		astepSuAvg+=fabs(e2 * sV[m][f]);
		astepSvAvg+=fabs(e2 * sU[u][f]);
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
		asY += fabs(sY[m][f]);
		astepSyAvg+=fabs(ycontrib[f]);
		n3++;
				}
			}
		}

		aErrAvg/=n1;
		astepSuAvg/=n2;
		astepSvAvg/=n2;
		astepSyAvg/=n3;
		abU/=n1, abV/=n1, asU/=n2, asV/=n2, asY/=n2;
		double bUREG = 1.9074 / 100.0 * aErrAvg / abU;
		double bVREG = 1.9074 / 100.0 * aErrAvg / abV;
		double sUREG = 1.9074 / 100.0 * astepSuAvg / asU;
		double sVREG = 1.9074 / 100.0 * astepSvAvg / asV;
		double sYREG = 1.9074 / 100.0 * astepSyAvg / asY;
		printf("NREG - bU: %f bV: %f, sU: %f, sV: %f, sY: %f\n", bUREG, bVREG, sUREG, sVREG, sYREG);

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

			for(j=0;j<d0;j++) {
				int m=userent[base0+j]&USER_MOVIEMASK;
				double ee = err[base0+j];
				double e2 = ee;
				e2 -= (bU[u] + bV[m]);
				for (f=0; f<NFEATURES; f++)
					e2 -= ( (sU[u][f] + lNuSY[f]) * sV[m][f]);
				//int r=(userent[base0+j]>>USER_LMOVIEMASK)&7;
				//r++;
				//double rui = r - e2;
				//if ( rui > 5.00 )
					//e2 += (rui-5.0);
				//else if (rui < 1.0)
					//e2 -= (1.0 - rui);

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
				int m=userent[base+i]&USER_MOVIEMASK;
				double e=err[base+i];
				e-=(bU[u] + bV[m]);
				for (f=0; f<NFEATURES; f++)
					e-=((sU[u][f] + lNuSY[f]) * sV[m][f]);
				//int r=(userent[base+i]>>USER_LMOVIEMASK)&7;
				//r++;
				//double rui = r - e;
				//if ( rui > 5.00 )
					//e += (rui-5.0);
				//else if (rui < 1.0)
					//e -= (1.0 - rui);
				s+=e*e;
			}
			n+=d;
		}
		nrmse=sqrt(nrmse/ntrain);
		prmse = sqrt(s/n);
		
		lg("%f\t%f\t%f\n",nrmse,prmse,(clock()-t0)/(double)CLOCKS_PER_SEC);
		Gamma1 *= 0.90;
		Gamma2 *= 0.90;
	}
	
	/* Perform a final iteration in which the errors are clipped and stored */
	removeUV();
	
	//if(save_model) {
		//dappend_bin(fnameV,sV,NMOVIES);
		//dappend_bin(fnameU,sU,NUSERS);
	//}
	
	return 1;
}
