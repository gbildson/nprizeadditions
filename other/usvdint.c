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
/* Facorization based estimation as described in section 4.3 of cf.pdf
* With time based weights
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

#define ALPHA (25.)
//#define E (0.0001) // stop condition
#define E (0.00008) // stop condition
//#define E2 (0.00003) // stop condition
#define E2 (0.00002) // stop condition
int score_argv(char **argv) {return 0;}



FILE *fpV=NULL, *fpU=NULL;
int moviecount[NMOVIES];
void score_setup()
{
	int i,u;
    weight_time_setup();
	if(load_model) {
		fpV=fopen(fnameV,"rb");
		fpU=fopen(fnameU,"rb");
		if(fpV || fpU) {
			lg("Loading %s and %s\n",fnameV,fnameU);
			if(!fpV || !fpU)
				error("Cant open both files");
		}
	}
	
	ZERO(moviecount);
	for(u=0;u<NUSERS;u++) {
		int base=useridx[u][0];
		int d=UNTRAIN(u);
		int i;
		for(i=0; i<d;i++) {
			int m=userent[base+i]&USER_MOVIEMASK;
			moviecount[m]++;
		}
	}
}

double sU[NUSERS],sV[NMOVIES];
double sY[NMOVIES];
#define L0 (0.005) // for biases
//#define L4 (0.02) // for biases
#define L4 (0.002) // for biases
#define L6 (0.005) // for biases
#define L2_ORIG (0.007) 
#define L2_FAST (0.8) 
#define L7 (0.015) 
//#define L7 (0.028) 
//#define L8 (0.035) 
#define L8 (0.015) 
double bU[NUSERS]; // moviebag
double bV[NMOVIES];
//float  sija[157877565];
float  wIJ[157877565];
float  cIJ[157877565];

int getOffset(int i, int j) {
	return i * (NMOVIES) - (i+1) * i / 2 + j - 1;
}

float getWIJ(int i, int j) {
	int loc = getOffset(i,j);
	return wIJ[loc];
}

float getCIJ(int i, int j) {
	int loc = getOffset(i,j);
	return cIJ[loc];
}


void removeUV()
{
	int u;
	for(u=0;u<NUSERS;u++) {
		int base0=useridx[u][0];
		int d012=UNALL(u);
		int i;

	    double NuS=1.0/sqrt(d012);
	    double sumY = 0.0;
	    for(i=0;i<d012;i++) {
		    int m=userent[base0+i]&USER_MOVIEMASK;
		    sumY+=sY[m];
	    }
	    double NuSY = NuS * sumY; 

        double bUu=bU[u];
		for(i=0; i<d012;i++) {
			int m=userent[base0+i]&USER_MOVIEMASK;
			err[base0+i]-=(sU[u]+NuSY)*sV[m] + bUu + bV[m];
		}
	}
}

double new_sV[NMOVIES];
//double new_sV[NMOVIES], new_sVV[NMOVIES];
double new_sY[NMOVIES];
double new_sU[NUSERS];
double lNuSY[NUSERS];
int score_train(int loop)
{
	int nfeatures=loop+1;
	if(fpV && fpU) {
		int nV=fread(sV,sizeof(double),NMOVIES,fpV);
		int nU=fread(sU,sizeof(double),NUSERS,fpU);
		
		if(!nV && !nU) {
			fclose(fpV);
			fclose(fpU);
			fpV=NULL;
			fpU=NULL;
		} else if(nV!=NMOVIES)
			error("Failed to read %s %d",fnameV,nV);
		else if(nU!=NUSERS)
			error("Failed to read %s %d",fnameU,nU);
		else {
			removeUV();
			return 1;
		}
	}
	
	/* Initial estimation for current feature */
	{
		int u,m;
		
		for(u=0;u<NUSERS;u++) {
			new_sU[u]=0.1;
			bU[u]=0.01;
			lNuSY[u]=0.0;
		}
		for(m=0;m<NMOVIES;m++) {
			new_sV[m]=0.05;
			bV[m]=0.0;
			new_sY[m]=0.0;
		}
	}
	
	/* Optimize current feature */
	double rmse=2., last_rmse=10.;
	double thr=sqrt(1.-E);
	int loopcount=0;
	int hitFastTarget=0;
	int badData=0;
	//if ( loop > 3 ) {
	    thr=sqrt(1.-E2);
	//}
	while( (rmse/last_rmse<thr && rmse < last_rmse ) || loopcount++ < 20) {
	    //int addonCount=0;
		double l2 = L2_ORIG;
		//if (!hitFastTarget) 
		if (loopcount<17) {
			l2 = L2_FAST;
		}
		int m;
		for(m=0;m<NMOVIES;m++) {
			sV[m]=new_sV[m];
			sY[m]=new_sY[m];
		}
		int u;
		for(u=0;u<NUSERS;u++) sU[u]=new_sU[u];
        double bvslope[NMOVIES];
		ZERO(bvslope);
        double yslope[NMOVIES];
		ZERO(yslope);
		
		last_rmse=rmse;
		clock_t t0=clock();
		ZERO(new_sV);
		for(u=0;u<NUSERS;u++) {
			int base0=useridx[u][0];
			int d0=UNTRAIN(u);
			double sUu=sU[u];
			double lNuSYu=lNuSY[u];
			double bUu=bU[u];
			int j;
			double e[NMOVIES];

			int dall=UNALL(u);
			double NuS=1.0/sqrt(dall);
			double sumY = 0.0;
			double ycontrib = 0.0;
			for(j=0;j<d0;j++) {
				int mm=userent[base0+j]&USER_MOVIEMASK;
				double sVm=sV[mm];
				double ee=err[base0+j];
				double e2 = ee-(sUu+lNuSYu)*sVm - bUu - bV[mm];
				e[j] = e2;
				//sumY+=sY[mm]*e2;
				sumY+=sY[mm];
				//yslope[mm]+=e2*NuS*sV[mm];
				ycontrib=e2*sVm;
			}
			double NuSY = NuS * sumY; 
			lNuSY[u] = NuSY;

			// Update the Y slope contributions for the N(u) set
			ycontrib*=NuS; // / d0???
			for(j=0;j<dall;j++) {
				int mm=userent[base0+j]&USER_MOVIEMASK;
				yslope[mm]+=ycontrib;
			}
			
			double t_new_sU=0.;
			for(j=0;j<d0;j++) {
				int mm=userent[base0+j]&USER_MOVIEMASK;
				double sVm=sV[mm];
				t_new_sU+=e[j]*sVm;
			}
			t_new_sU/=d0;
			t_new_sU=sUu +l2*(t_new_sU -L8*sUu);
			new_sU[u]=t_new_sU;

			for(j=0;j<d0;j++) {
				int mm=userent[base0+j]&USER_MOVIEMASK;
				//new_sV[mm]+=(e[j]*(t_new_sU+NuSY));
				//new_sV[mm]+=(e[j]*(sUu+NuSY));
				new_sV[mm]+=(e[j]*(sUu+lNuSYu));
			}
		}
		for(m=0;m<NMOVIES;m++) {
			double sVm=sV[m];
			int mvcnt=moviecount[m];
			new_sV[m]/=mvcnt;
			new_sV[m]=sVm + l2*(new_sV[m] - L8*sVm);
			new_sY[m]+=l2*(yslope[m]/mvcnt - L7*sY[m]);
		}

		rmse=0.;
		int ntrain=0;
		for(u=0;u<NUSERS;u++) {
			double bUu=bU[u];
			double buslope=0;
			int base0=useridx[u][0];
			int d0=UNTRAIN(u);
			int j;

			double NuSY = lNuSY[u];

			for(j=0;j<d0;j++) {
				int m=userent[base0+j]&USER_MOVIEMASK;
				double bVm=bV[m];
				double ee = err[base0+j];
				double e = err[base0+j]-(new_sU[u]+NuSY)*new_sV[m] - bUu - bVm;
if( e > 5.0 || e < -5.0 ) {
    printf("breaking EE: %f\tU: %d\tM: %d\tNuSY: %f\te: %f\t sV: %f\tsU: %f\tbU: %f\tbV: %f\n", ee, u, m, NuSY, e, new_sV[m], new_sU[u], bUu, bVm);
	fflush(stdout);
	badData = 1;
	break;
}

if( e > 5.0 || e < -5.0 ) {
    printf("EE: %f\tU: %d\tM: %d\tNuSY: %f\te: %f\t sV: %f\tsU: %f\tbU: %f\tbV: %f\n", ee, u, m, NuSY, e, new_sV[m], new_sU[u], bUu, bVm);
	fflush(stdout);
}
//if (!hitFastTarget && loopcount > 6 ) {
    //double addon = -(new_sU[u]+NuSY)*new_sV[m] - bUu - bVm;
    //if (addon > 1.4 || addon < -1.4 )
		//addonCount++;
    //if (addonCount > 2000) {
		//hitFastTarget = 1;
    	//l2 = L2_ORIG;
    	//l2 /= 2;
        //printf("Target U: %d\tM: %d\tNuSY: %f\te: %f\t addon:%f\n", u, m, NuSY, e-addon, addon);
        //fflush(stdout);
	//}
//}
//printf("U: %d\tM: %d\tNuSY: %f\te: %f\t addon:%f\n", u, m, NuSY, e-addon, addon);
//fflush(stdout);
//}
//if ( e > 4.5 || e < -4.5 ) {
//double addon = (new_sU[u]+NuSY)*new_sV[m] - bUu - bVm;
//printf("U: %d\tM: %d\tNuSY: %f\te: %f\t addon:%f\n", u, m, NuSY, e, addon);
//fflush(stdout);
//}

				rmse+=e*e;

				buslope+=(e-bUu*L4);
				bvslope[m]+=(e-bVm*L4);
			}
			if (badData) break;
		
			ntrain+=d0;
        	buslope*=(L0/d0);
			bU[u] += buslope;
		}
		if (badData) break;
		rmse=sqrt(rmse/ntrain);
		
		for(m=0;m<NMOVIES;m++) 
		    bV[m] += L0*bvslope[m]/moviecount[m];
		
		lg("%f %f\r",rmse,(clock()-t0)/(double)CLOCKS_PER_SEC);
	}
	
	/* Perform a final iteration in which the errors are clipped and stored */
	removeUV();
	
	if(save_model) {
		dappend_bin(fnameV,sV,NMOVIES);
		dappend_bin(fnameU,sU,NUSERS);
	}
	
	return 1;
}
