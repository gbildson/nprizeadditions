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
#define NUM_FEATURES (30) 
#define ALPHA (1.0) 
#define GAMMA (0.08) 

static char *fnameVNS1="data/nsim1V.bin";
static char *fnameWNS1="data/nsvd1W.bin";
//double sV[NMOVIES];
double sVall[NMOVIES*NUM_FEATURES];
//double dbuf[NMOVIES*2000];
float  sija[157877565];

int sFcount = 0;
int sijcount = 0;
int simc = 0;
double *sV;
double sW[NMOVIES];
double sU[NUSERS]; // moviebag
double unorm[NUSERS];
	
int score_argv(char **argv) {return 0;}
void computeSIJ(int i, int j);
double getSIJ(int i, int j);
void computeQFactors();
void computeAllSIJ();

FILE *fpV=NULL,*fpW=NULL;
void score_setup()
{

	if(load_model) {
		fpV=fopen(fnameVNS1,"rb");
		if(!fpV)
			error("Cant open V files");
		else
			lg("Loading %s\n",fnameVNS1);
		fpW=fopen(fnameWNS1,"rb");
		if(!fpW)
			error("Cant open W files");
		else
			lg("Loading %s\n",fnameWNS1);
		//if(fpV || fpW) {
			//lg("Loading %s and %s\n",fnameVNS1,fnameWNS1);
			//if(!fpV || !fpW)
				//error("Cant open both files");
		//}
	    if(fpV /*&& fpW*/) {
			int nV=fread(sVall,sizeof(double),NMOVIES*NUM_FEATURES,fpV);
			
			if(!nV) {
				fclose(fpV);
				fpV=NULL;
			} else if(nV!=NMOVIES*NUM_FEATURES)
				error("Failed to read %s %d",fnameVNS1,nV);
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
	if(fpW) {
		int nW=fread(sW,sizeof(double),NMOVIES,fpW);
		if(!nW) {
			fclose(fpW);
			fpW=NULL;
		} else if(nW!=NMOVIES)
			error("Failed to read %s %d",fnameWNS1,nW);
	}
	/*if(fpV && fpW) {
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
	}*/

	//if (load_model) {
		sV = &(sVall[loop*NMOVIES]);
	//}
	
	/* Initial estimation for current feature */
	if(!fpW) {
		int m;
		for(m=0;m<NMOVIES;m++) {
			if (!load_model)
			    sV[m]=0.1;
			sW[m]=0.1;
		}
	}

	computeQFactors();
	computeAllSIJ();

	int loopc = 0;
	
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
			ZERO(vslope);
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
				double uslope=0;

				//// Precompute sij factors
				//double sadjust, sijoffset, sijsum;
				////double *sijs=malloc(d0*d0*sizeof(double));
				//float *sijs=sij;
				////if ( d0*d0 > NMOVIES*2000)
					////error("too many sij");
				//double *sij;
				//double sijv, sijterm;
				//unsigned int *ent2=ent;
				//unsigned int *ent3;
				//int r, isub, jsub, j2, firstpass=1;
				//sijsum = 0.0;
				//int maxd0 = d0;
				//if ( maxd0 > 5950 )
					//maxd0 = 5950;
				//for(j=0;j<maxd0;j++) {
					//int m=(*ent2++)&USER_MOVIEMASK;
				    //ent3 = &(*ent2);
				    ////sij  = &(*(sijs+j*maxd0+j+1));
				    //sij  = (sijs+j*maxd0+j+1);
				    //for(j2=j+1;j2<maxd0;j2++) {
					    //int m2=(*ent3++)&USER_MOVIEMASK;
						//sijv = computeSIJ(m,m2);
						//*sij++ = sijv;
						//sijsum += sijv;
						////printf("sij(%d,%d) = %.6f\n", m,m2,sijv);
					//}
				//}
				////printf("sijsum = %.6f\n", sijsum);
				////printf("sij mem: %d\n",d0*d0);

				// Precompute sij factors
				double sadjust, sijoffset, sijsum;
				//double *sijs=malloc(d0*d0*sizeof(double));
				//float *sijs=sij;
				//double *sij;
				double sijv, sijterm;
				unsigned int *ent2=ent;
				unsigned int *ent3;
				int r, isub, jsub, j2, firstpass=1;
				sijsum = 0.0;
				//int maxd0 = d0;
				//if ( maxd0 > 5950 )
				//	maxd0 = 5950;
				for(j=0;j<d0;j++) {
					int m=(*ent2++)&USER_MOVIEMASK;
				    ent3 = &(*ent2);
				    //sij  = &(*(sijs+j*maxd0+j+1));
				    //sij  = (sijs+j*maxd0+j+1);
				    for(j2=j+1;j2<d0;j2++) {
					    int m2=(*ent3++)&USER_MOVIEMASK;
						sijv = getSIJ(m,m2);
						//*sij++ = sijv;
						sijsum += sijv;
						//printf("sij(%d,%d) = %.6f\n", m,m2,sijv);
					}
				}
				//printf("sijsum = %.6f\n", sijsum);
				//printf("sij mem: %d\n",d0*d0);

				for(j=0;j<d0;j++) {
					int m=(*ent++)&USER_MOVIEMASK;
					double sVm=sV[m];

					// Compute similarity offsets
					//if (firstpass) {
					sijoffset = 0.0;
				    ent3 = &userent[base0];
				    for(j2=0;j2<d0;j2++) {
						if (j2 == j) continue;
						//if (j < j2) {
							//isub = j;
							//jsub = j2;
						//} else {
							//isub = j2;
							//jsub = j;
						//}
						//sijv = *(sijs+isub*maxd0+jsub);
					    int m2=(*ent3)&USER_MOVIEMASK;
					    r=(*ent3++>>USER_LMOVIEMASK)&7;
						sijv = getSIJ(m,m2);
						sijterm = sijv*(sUu*sV[m2] - r);
						sijoffset += sijterm;
						//printf("sijterm(%d,%d) = %.6f | sijv = %.6f \n", m,m2,sijterm, sijv);
						//simc++;
					}
					if ( sijsum == 0.0 ) 
					    sadjust = 0.0;
					else
						sadjust = GAMMA * sijoffset / sijsum;
					//}


					double facts = sUu*sVm;
					double t = (*e++) - facts - sadjust; // New estimation error
					//if (firstpass) {
					if ( loopc++ % 40000 == 19999 ) {
				    printf("sadjust = %.4f | off = %.4f | sum = %.4f | facts = %.4f | t = %4f | sU = %.4f | sV = %.4f | u=%d | m=%d\n", sadjust, sijoffset, sijsum, facts, t, sUu, sVm, u,m);
					fflush(stdout);
						loopc = 0;
					}
				    //printf("sicount = %d | sFcount = %d | simc %d\n", sijcount, sFcount, simc);
					//sijcount=0;
					//sFcount=0;
					//simc=0;
					//}
					//firstpass = 0;

					rmse+=t*t;
					vslope[m]+=t*sUu;
					uslope+=t*sVm;
//if ( (uucount++ % 1000000) == 0 ) {
//printf("%d v bef:%f\tdsyg:%f\tu bef:%f t:%f vslope:%f\n", uucount,
                    //sUu,
                    //sUu,
                    //sVm, t, t*(sUu));
//}
				}
				//free(sijs);
				ntrain+=d0;
				uslope*=unorm[u];
				ent=&userent[base0];
				d0=UNTOTAL(u);
				for(j=0;j<d0;j++)
					wslope[(*ent++)&USER_MOVIEMASK]+=uslope;
			}
			rmse=sqrt(rmse/ntrain);
//printf("RMSE = %0.5f\n", rmse);
			/* early stopping */
			if(rmse>last_rmse) break;
			last_rmse=rmse;
			
			int m;
			for(m=0;m<NMOVIES;m++) {
				sV[m] += LRATE*vslope[m] - LRATE_LAMBDA*sV[m];
				sW[m] += LRATE*wslope[m] - LRATE_LAMBDA*sW[m];
			}
			lg("%d %f %f\t\r",epoch,rmse,(clock()-t0)/(double)CLOCKS_PER_SEC);
		}
	}
	
	/* Perform a final iteration in which the errors are clipped and stored */
	removeUV();
	
	/* save results */
	if(save_model) {
		dappend_bin(fnameVNS1,sV,NMOVIES);
		//dappend_bin(fnameWNS1,sW,NMOVIES);
	}	
	return 1;
}

//double sij[NMOVIES*8000];
float sVFact[NMOVIES];
void computeQFactors()
{
	double qsumi, qsumj, qdiff, qsqsum;
	double qki, qkj;

	int k, koff, i;
	for (i=0; i < NMOVIES; i++) {
		for (k=0; k < NUM_FEATURES; k++) {
			koff = k*NMOVIES;
			qki = sVall[koff+i];
			qsumi += qki*qki;
			sFcount++;
		}
		sVFact[i] = (float)sqrt(qsumi);
	}
}

void computeAllSIJ()
{
	int j, i;
	for (i=0; i < NMOVIES; i++) {
		for (j=i+1; j < NMOVIES; j++) {
			computeSIJ(i,j);
		}
	}
}

void computeSIJ(int i, int j)
{
	double /*qsumi, qsumj,*/ qdiff, qsqsum=0.0, qterm;
	double qki, qkj;

	int k, koff;
	for (k=0; k < NUM_FEATURES; k++) {
		koff = k*NMOVIES;
		qki = sVall[koff+i];
		qkj = sVall[koff+j];
		//qsumi += qki*qki;
		//qsumj += qkj*qkj;
		qdiff = qki - qkj;
		qterm = qdiff*qdiff;
		qsqsum += qterm;
		//printf("qDiffSq = %.6f\n", qterm);
		//sijcount++;
	}
	
	//double sij = qsqsum / (sqrt(qsumi)*sqrt(qsumj));

	double sij = qsqsum / ((double)((sVFact[i]*sVFact[j])));
	int loc = i * (NMOVIES) - (i+1) * i / 2 + j - 1;
	sija[loc] = (float)sij;

	// sij = pow(sij, -ALPHA); // ALPHA=1

	//printf("sij = %.6f | qsqsum = %.6f | qIs = %.6f | qJs = %.6f\n", sij, qsqsum, sVFact[i], sVFact[j]);

	//return sij;
}

double getSIJ(int i, int j){
	if ( j < i ) {
		int temp = j;
		j = i;
		i = temp;
	} else if (i == j) {
		return 0.01;
	}
	int loc = i * (NMOVIES) - (i+1) * i / 2 + j - 1;
	return (double)sija[loc];
}

