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
static char *fnameV="data/usvdbkV.bin";
static char *fnameU="data/usvdbkU.bin";

#define ALPHA (25.)
#define E (0.0001) // stop condition
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
#define L4 (0.002) // for biases
#define L2 (0.007) 
#define L7 (0.015) 
double bU[NUSERS]; // moviebag
double bV[NMOVIES];

void removeUV()
{
	int u;
	for(u=0;u<NUSERS;u++) {
		int base0=useridx[u][0];
		int d012=UNALL(u);
		int i;
        double bUu=bU[u];
		for(i=0; i<d012;i++) {
			int m=userent[base0+i]&USER_MOVIEMASK;
			err[base0+i]-=sU[u]*sV[m] + bUu + bV[m];
		}
	}
}

double new_sV[NMOVIES], new_sVV[NMOVIES];
double new_sY[NMOVIES];
double new_sU[NUSERS];
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
			bU[u]=0.0;
		}
		for(m=0;m<NMOVIES;m++) {
			new_sV[m]=0.1;
			bV[m]=0.0;
			new_sY[m]=0.0;
		}
	}
	
	/* Optimize current feature */
	double rmse=2., last_rmse=10.;
	double thr=sqrt(1.-E);
	while(rmse/last_rmse<thr) {
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
		ZERO(new_sVV);
		for(u=0;u<NUSERS;u++) {
			int base0=useridx[u][0];
			int d0=UNTRAIN(u);
			double NuS=sqrt(UNALL(u));
			double sumY = 0.0;
			int j;
			double e[NMOVIES];
			double w[NMOVIES];
			int m[NMOVIES];
			for(j=0;j<d0;j++) {
				int mm=userent[base0+j]&USER_MOVIEMASK;
				m[j]=mm;
				double ee=err[base0+j];
				int nuv=moviecount[mm];
				sumY+=sY[mm]*ee/nuv;
				yslope[mm]+=ee*NuS*sV[mm];
				if(d0<nuv) nuv=d0;
				w[j]=wgt[base0+j];
				e[j]=w[j]*ee*nuv/(nuv+ALPHA*nfeatures);
			}
			double NuSY = NuS * sumY; 
			
			double t_new_sU=0., new_sUU=0.;
			for(j=0;j<d0;j++) {
				double sVm=sV[m[j]];
				t_new_sU+=e[j]*sVm;
				new_sUU+=w[j]*sVm*sVm;
			}
			t_new_sU/=new_sUU;
			new_sU[u]=t_new_sU;

			double sUusUu=t_new_sU*t_new_sU;
			for(j=0;j<d0;j++) {
				int mm=m[j];
				new_sV[mm]+=e[j]*t_new_sU+NuSY;
				new_sVV[mm]+=w[j]*sUusUu;
			}
		}
		for(m=0;m<NMOVIES;m++) {
			new_sV[m]/=new_sVV[m];
			new_sY[m]+=L2*(yslope[m]/moviecount[m] - L7*sY[m]);
		}

		rmse=0.;
		int ntrain=0;
		for(u=0;u<NUSERS;u++) {
			double bUu=bU[u];
			double buslope=0;
			int base0=useridx[u][0];
			int d0=UNTRAIN(u);
			int j;

			double NuS=sqrt(UNALL(u));
			double sumY = 0.0;
			for(j=0;j<d0;j++) {
				int m=userent[base0+j]&USER_MOVIEMASK;
				sumY+=new_sY[m];
			}
			double NuSY = NuS * sumY; 

			for(j=0;j<d0;j++) {
				int m=userent[base0+j]&USER_MOVIEMASK;
				double bVm=bV[m];
				double e = err[base0+j]-(new_sU[u]+NuSY)*new_sV[m] - 3.7 - bUu - bVm;
				rmse+=e*e;

				buslope+=e-bUu*L4;
				bvslope[m]+=e-bVm*L4;
			}
			ntrain+=d0;
        	buslope*=(L0/d0);
			bU[u] += buslope;
		}
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
