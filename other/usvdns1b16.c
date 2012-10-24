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

#define MAX_EPOCHS (120) // Maximal number of training iterations per feature
// Training parameters http://www.netflixprize.com/community/viewtopic.php?pid=6278#p6278
#define LRATE (0.0001) // Rate of learning 
#define LAMBDA (0.002) // Regularization factor
#define LAMBDA2 (0.05) // factor for constants
#define LRATE_LAMBDA (LAMBDA*LRATE) // (LRATE*LAMBDA)
#define L0 (0.005) // for biases
#define L4 (0.002) // for biases

static char *fnameVNS1="data/nsvd1V.bin";
static char *fnameWNS1="data/nsvd1W.bin";
double sV[NMOVIES];
double sW[NMOVIES];
double sU[NUSERS]; // moviebag
double bU[NUSERS]; // moviebag
double bV[NMOVIES];
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
		double bUu=bU[u];
		for(i=0; i<d012;i++) {
			int m=ent[i]&USER_MOVIEMASK;
			err[base0+i]-=(sUu*sV[m] + bV[m] + bUu);
		}
	}
}

int score_train(int loop)
{
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
			bV[m]=0.0;
		}
		int u;
		for(u=0;u<NUSERS;u++) {
			bU[u]=0.0;
		}
	}
	
	/* Optimize current feature */
	{
		int epoch;
		double rmse, last_rmse;
		for(last_rmse=1.e20,epoch=0; epoch<MAX_EPOCHS; epoch++) {
			clock_t t0=clock();
			computeU();
			rmse=0.;
			int u;
			int ntrain=0;
			double vslope[NMOVIES];
			double bvslope[NMOVIES];
			int vcount[NMOVIES];
			ZERO(vslope);
			ZERO(bvslope);
			ZERO(vcount);
			double wslope[NMOVIES];
			ZERO(wslope);
//int uucount=0;
			for(u=0;u<NUSERS;u++) {
				int base0=useridx[u][0];
				float *e=&err[base0];
				unsigned int *ent=&userent[base0];
				int d0=UNTRAIN(u);
				int j;
				double s=0.;
				double sUu=sU[u];
				double bUu=bU[u];
				double uslope=0;
				double buslope=0;
				for(j=0;j<d0;j++) {
					int m=(*ent++)&USER_MOVIEMASK;
					double sVm=sV[m];
					double bVm=bV[m];
					double t = (*e++) - sUu*sVm - bVm - bUu; // New estimation error
					rmse+=t*t;
					vslope[m]+=t*sUu;
					uslope+=t*sVm;
					buslope+=t-bUu*L4;
					bvslope[m]+=t-bVm*L4;
					vcount[m]++;
//if ( (uucount++ % 1000000) == 0 ) {
//printf("%d v bef:%f\tdsyg:%f\tu bef:%f t:%f vslope:%f\n", uucount,
                    //sUu,
                    //sUu,
                    //sVm, t, t*(sUu));
//}
				}
				ntrain+=d0;
				uslope*=unorm[u];
				buslope*=unorm[u]*L0;
				bU[u] += buslope;
				ent=&userent[base0];
				d0=UNTOTAL(u);
				for(j=0;j<d0;j++)
					wslope[(*ent++)&USER_MOVIEMASK]+=uslope;
			}
			rmse=sqrt(rmse/ntrain);
			/* early stopping */
			if(rmse>last_rmse) break;
			last_rmse=rmse;
			
			int m;
			for(m=0;m<NMOVIES;m++) {
				sV[m] += LRATE*vslope[m] - LRATE_LAMBDA*sV[m];
				sW[m] += LRATE*wslope[m] - LRATE_LAMBDA*sW[m];
				bV[m] += L0*bvslope[m]/vcount[m];
			}
			lg("%d %f %f\t\r",epoch,rmse,(clock()-t0)/(double)CLOCKS_PER_SEC);
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
