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
/*
* Regular-Paterek.pdf NSVD1 described in section 3.8
* also see http://www.netflixprize.com/community/viewtopic.php?id=898
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "basic.h"
#include "netflix.h"
#include "utest.h"

#define MAX_EPOCHS (66) // Maximal number of training iterations per feature
// Training parameters http://www.netflixprize.com/community/viewtopic.php?pid=6278#p6278
#define LRATE_ORIG (0.0001) // Rate of learning 
#define LAMBDA (0.002)// Regularization factor
#define LRATE_LAMBDA_ORIG (LAMBDA*LRATE_ORIG) // (LRATE*LAMBDA)
//#define LAMBDA2 (0.05) // factor for constants
//#define LAMBDA_ORIG (0.002) // Regularization factor

static char *fnameVNS1="data/nsvd109V.bin";
static char *fnameWNS1="data/nsvd109W.bin";
double sV[NMOVIES];
double sW[NMOVIES];
double sU[NUSERS]; // moviebag
//int    sUCnt[NUSERS]; // moviebag
int sVCnt[NMOVIES];
double unorm[NUSERS];
	
int score_argv(char **argv) {return 0;}

FILE *fpV=NULL,*fpW=NULL;
void score_setup()
{
	if(load_model) {
		fpV=fopen(fnameVNS1,"rb");
		fpW=fopen(fnameWNS1,"rb");
		if(fpV || fpW) {
			lg("Loading %s and %s\n",fnameVNS1,fnameWNS1);
			if(!fpV || !fpW)
				error("Cant open both files");
		}
	}
	int u;
	for(u=0;u<NUSERS;u++)
		unorm[u]=1./(UNTOTAL(u)+1.); // sqrt
}

void computeU()
{
	ZERO(sU);
	int u;
	for(u=0;u<NUSERS;u++) {
		int base0=useridx[u][0];
		int d012=UNTOTAL(u); // not UNALL
		int i;
		unsigned int *ent=&userent[base0];
		double sUu=0.;
		for(i=0; i<d012;i++)
			sUu+=sW[ent[i]&USER_MOVIEMASK];
		sU[u]=sUu*unorm[u];
	}
}

void removeUV()
{
	computeU();
	int u;
	for(u=0;u<NUSERS;u++) {
		int base0=useridx[u][0];
		unsigned int *ent=&userent[base0];
		int d012=UNALL(u);
		int i;
		double sUu=sU[u];
		for(i=0; i<d012;i++) {
			err[base0+i]-=sUu*sV[ent[i]&USER_MOVIEMASK];
		}
	}
}

int score_train(int loop)
{
    double lrate=0.0001;
    double lrate_lambda=LAMBDA*lrate; 
	double penalty = (0.88 + ((double)loop)/100.0);
	if ( penalty > 1.0 )
		penalty = 1.0;

	double rmse_targ;
	double vcavg=0.0;
	{
		int count=0;
		int u;
		double rmse=0.0;
		int m;
		for(m=0;m<NMOVIES;m++)
			sVCnt[m] = 0;
		for(u=0;u<NUSERS;u++) {
			int base0=useridx[u][0];
			float *e=&err[base0];
			unsigned int *ent=&userent[base0];
			int d0=UNTRAIN(u);
			int j;
			for(j=0;j<d0;j++) {
				m=(*ent++)&USER_MOVIEMASK;
				double t = (*e++);
				rmse+=t*t;
				count++;
				//sUCnt[u]++;
				sVCnt[m]++;
				vcavg++;
			}
		}
		rmse=sqrt(rmse/count);
		vcavg /= NMOVIES;
		int i;
		double goal = 3.5;
//printf("GOAL: %f\n", goal);
		for (i=0; i< loop; i++)
			goal *= 0.80;
		goal = 1.0 - goal/1000.0;
printf("GOAL: %f\n", goal);
	    rmse_targ = rmse*goal;
printf("RMSE GOAL: %f\n", rmse_targ);
		/*
		if ( loop < 4 ) {
	        rmse_targ = rmse*0.997;
		} else if ( loop < 6 ) {
	        rmse_targ = rmse*0.997;
		} else if ( loop < 12 ) {
	        rmse_targ = rmse*0.998;
		} else if ( loop < 18 ) {
	        rmse_targ = rmse*0.9985;
		} else {
	        rmse_targ = rmse*0.999;
		}
		*/
		//if ( loop > 0 ) {
       		//lrate_lambda = LRATE_LAMBDA_ORIG/(loop+19.0/20.0);
       		//lrate = LRATE_ORIG/(loop+19.0/20.0);
		//}
	}

	if(fpV && fpW) {
		int nV=fread(sV,sizeof(double),NMOVIES,fpV);
		int nW=fread(sW,sizeof(double),NMOVIES,fpW);
		
		if(!nV && !nW) {
			fclose(fpV);
			fclose(fpW);
			fpV=NULL;
			fpW=NULL;
		} else if(nV!=NMOVIES)
			error("Failed to read %s %d",fnameVNS1,nV);
		else if(nW!=NMOVIES)
			error("Failed to read %s %d",fnameWNS1,nW);
		else {
			removeUV();
			return 1;
		}
	}
	
	/* Initial estimation for current feature */
	{
		int m;
		for(m=0;m<NMOVIES;m++) {
			sV[m]=0.1;
			sW[m]=0.1;
		}
	}
	
	/* Optimize current feature */
	{
		int epoch;
		double rmse, last_rmse;
		double uavg=0.0, vavg=0.0;
		for(last_rmse=1.e20,epoch=0; epoch<MAX_EPOCHS; epoch++) {
			clock_t t0=clock();
			computeU();
			rmse=0.;
			int ntrain=0;
			double vslope[NMOVIES];
			ZERO(vslope);
			double wslope[NMOVIES];
			ZERO(wslope);

			int u;
			for(u=0;u<NUSERS;u++) {
				uavg += sU[u];
			}
			uavg /= NUSERS;
			int mm;
			for(mm=0;u<NMOVIES;mm++) {
				vavg += sV[mm];
			}
			vavg /= NMOVIES;
			
//int uucount=0;
			for(u=0;u<NUSERS;u++) {
				int base0=useridx[u][0];
				float *e=&err[base0];
				unsigned int *ent=&userent[base0];
				int d0=UNTRAIN(u);
				int j;
				double s=0.;
				double sUu=sU[u];
				double lsUu = sUu;
				//lsUu += (uavg-sUu) * 0.05;
				double uslope=0;
				for(j=0;j<d0;j++) {
					int m=(*ent++)&USER_MOVIEMASK;
					double sVm=sV[m];
					//sVm += (vavg-sVm) * 0.05;
					double ssVm;
					int svcnt = sVCnt[m];
					if ( svcnt > vcavg*100 ) 
					    ssVm = sVm / 4.0;
					else if ( svcnt > vcavg*10 ) 
					    ssVm = sVm ;
					else
					    ssVm = sVm*4.0;
					
					//double t = penalty*(*e++) - sUu*sVm; // New estimation error
					double t = (*e++) - lsUu*sVm; // New estimation error
					rmse+=t*t;
					vslope[m]+=t*lsUu;
					uslope+=t*ssVm;
//if ( (uucount++ % 1000000) == 0 ) {
//printf("%d v bef:%f\tdsyg:%f\tu bef:%f t:%f vslope:%f\n", uucount,
                    //sUu,
                    //sUu,
                    //sVm, t, t*(sUu));
//}
				}
				ntrain+=d0;
				uslope*=unorm[u];
				ent=&userent[base0];
				d0=UNTOTAL(u);
				for(j=0;j<d0;j++)
					wslope[(*ent++)&USER_MOVIEMASK]+=uslope;
			}
			rmse=sqrt(rmse/ntrain);
			/* early stopping */
			if( epoch >= 18 && rmse>last_rmse) break;
			
			int m;
			for(m=0;m<NMOVIES;m++) {
				sV[m] += lrate*vslope[m] - lrate_lambda*sV[m];
				sW[m] += lrate*wslope[m] - lrate_lambda*sW[m];
			}
			lg("%d %f %f\t\r",epoch,rmse,(clock()-t0)/(double)CLOCKS_PER_SEC);

			double deltar = last_rmse-rmse;
			if( epoch >= 27 && (deltar < ((1.0/penalty)*0.0000001) || rmse <= rmse_targ) ) break;
			//if( epoch > 10 && deltar < ((1.0/penalty)*0.000002) ) break;
			last_rmse=rmse;
		}
	}
	
	/* Perform a final iteration in which the errors are clipped and stored */
	removeUV();
	
	/* save results */
	if(save_model) {
		dappend_bin(fnameVNS1,sV,NMOVIES);
		dappend_bin(fnameWNS1,sW,NMOVIES);
	}	
	return 1;
}
