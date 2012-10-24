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
//#define E (0.0001) // stop condition
//#define E (0.00008) // stop condition
//#define E2 (0.00003) // stop condition
#define E2 (0.00002) // stop condition
int score_argv(char **argv) {return 0;}
extern double rmse(int k);

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


#define NFEATURES (50)
#define GLOBAL_MEAN (3.6033)
double sU[NUSERS][NFEATURES];
double sV[NMOVIES][NFEATURES];
double sY[NMOVIES][NFEATURES];
double bU[NUSERS]; 
double bV[NMOVIES];
//float  sija[157877565];
//float  wIJ[157877565];
//float  cIJ[157877565];
float  wIJ[157895333];
float  cIJ[157895333];


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



#define G0 (0.005) // for biases
//#define L4 (0.02) // for biases
#define L4 (0.002) // for biases
#define L6 (0.005) // for biases
#define G2 (0.007) 
#define G2_ORIG (0.007) 
#define G2_FAST (0.8) 
#define L7 (0.015) 
//#define G2_Y (0.8) 
//#define L7_Y (0.015) 
//#define L7 (0.028) 
//#define L8 (0.035) 
#define L8 (0.015) 

double rmseprobe()
{
	int k=2;
    int u, f;
    int n=0;
    double s=0.;
    int i;

	for(u=0;u<NUSERS;u++) {
		int base=useridx[u][0];
		int base0=useridx[u][0];

		int dall=UNALL(u);
		double NuS = 1.0/sqrt(dall);
		double lNuSY[NFEATURES];
		double sumY[NFEATURES];
		ZERO(sumY);
		ZERO(lNuSY);
		int j, j2;
		for(j=0;j<dall;j++) {
			int mm=userent[base0+j]&USER_MOVIEMASK;
			for(f=0;f<NFEATURES;f++)
				sumY[f]+=sY[mm][f];
		}
		int d0=UNTRAIN(u);
		//for(j=0;j<d0;j++) {
			//int mm=userent[base0+j]&USER_MOVIEMASK;
			for(f=0;f<NFEATURES;f++) 
				lNuSY[f] = NuS * sumY[f]; 
		//}


		for(i=1;i<k;i++) base+=useridx[u][i];
		int d=useridx[u][k];
		for(i=0; i<d;i++) {
			int m=userent[base+i]&USER_MOVIEMASK;
			double rbw = 0.0;
			double sumC = 0.0;
			double RuS = 1.0/sqrt(d0);
			for(j2=0;j2<d0;j2++) {
				int mm=userent[base0+j2]&USER_MOVIEMASK;
				if (m != mm) {
					double ee=err[base0+j2];
					double rb=(ee - bU[mm]);
					int loc = getOffset(m,mm);
					rbw += rb*wIJ[loc];
					sumC += cIJ[loc];
				}
			}
			double e=err[base+i];
			e-=(bU[u] + bV[m]);
		    e -= RuS * rbw;
		    e -= NuS * sumC;
			for (f=0; f<NFEATURES; f++)
			    e-=((sU[u][f] + lNuSY[f]) * sV[m][f]);
			s+=e*e;
		}
		n+=d;
	}
    return sqrt(s/n);
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
		int j,j2;
		for(j=0;j<dall;j++) {
			int mm=userent[base0+j]&USER_MOVIEMASK;
			for(f=0;f<NFEATURES;f++)
				sumY[f]+=sY[mm][f];
		}
		int d0=UNTRAIN(u);
		//for(j=0;j<d0;j++) {
			//int mm=userent[base0+j]&USER_MOVIEMASK;
			for(f=0;f<NFEATURES;f++) 
				lNuSY[f] = NuS * sumY[f]; 
		//}

        double bUu=bU[u];
		for(i=0; i<d012;i++) {
			int m=userent[base0+i]&USER_MOVIEMASK;

			// Compute (ruj - buj) * wij and sumC
			//double rbw[NFEATURES];
			//ZERO(rbj);
			double rbw = 0.0;
			double sumC = 0.0;
			double RuS = 1.0/sqrt(d0);
			for(j2=0;j2<d0;j2++) {
				int mm=userent[base0+j2]&USER_MOVIEMASK;
				if (m != mm) {
					double ee=err[base0+j2];
					double rb=(ee - bU[mm]);
					int loc = getOffset(m,mm);
					rbw += rb*wIJ[loc];
					sumC += cIJ[loc];
				}
			}

			err[base0+i]-=(bU[u] + bV[m] + RuS * rbw + NuS * sumC);
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
			bU[u]=drand48()*0.01-0.005;
		}
		for(m=0;m<NMOVIES;m++) {
			bV[m]=drand48()*0.01-0.005;
		}
	}
	
	
	/* Initial estimation for current feature */
	{
		int u,m,f;
		
		for(u=0;u<NUSERS;u++) {
			for(f=0;f<NFEATURES;f++)
			    sU[u][f]=drand48()*0.1-0.04;
		}
		for(m=0;m<NMOVIES;m++) {
			for(f=0;f<NFEATURES;f++) {
			    sV[m][f]=drand48()*0.05-0.025;
			    sY[m][f]=drand48()*0.02-0.01;
			}
		}

		int i, j, loc, maxloc;
		for(i=0;i<NMOVIES;i++) {
			for(j=0;j<NMOVIES;j++) {
				loc = getOffset(i,j);
				if ( loc > maxloc ) 
					maxloc = loc;
				wIJ[loc] =drand48()*0.02-0.01;
				cIJ[loc] =drand48()*0.02-0.01;
			}
		}
		printf("done init maxloc=%d\n", maxloc);
		fflush(stdout);
	}
	
	/* Optimize current feature */
	double nrmse=2., last_rmse=10.;
	double thr=sqrt(1.-E);
	int loopcount=0;
	    //thr=sqrt(1.-E2);
	double Gamma2 = G2;
	double Gamma0 = G0;
	while( ( (nrmse < (last_rmse-E) ) || loopcount++ < 15) && loopcount < 27 ) {
		last_rmse=nrmse;
		clock_t t0=clock();

		int u,m, f;
		for(u=0;u<NUSERS;u++) {

			if (u%10000 == 0) {
				printf("On user: %d\n", u);
				fflush(stdout);
			}

			// Calculate sumY and NuSY for each factor
			double sumY[NFEATURES];
			ZERO(sumY);
			double lNuSY[NFEATURES];
			ZERO(lNuSY);
			int base0=useridx[u][0];
			int d0=UNTRAIN(u);
			int j, j2;
			int f;
			int dall=UNALL(u);
			double NuS = 1.0/sqrt(dall);
			for(j=0;j<dall;j++) {
				int mm=userent[base0+j]&USER_MOVIEMASK;
				for(f=0;f<NFEATURES;f++)
					sumY[f]+=sY[mm][f];
			}
//if ( loopcount > 1 ) {
//printf("sumY: %f\n", sumY);
//fflush(stdout);
//}
			//for(j=0;j<d0;j++) {
				//int mm=userent[base0+j]&USER_MOVIEMASK;
				for(f=0;f<NFEATURES;f++) {
					lNuSY[f] = NuS * sumY[f]; 
//if ( loopcount > 1 ) {
//printf("lNuSY: %f\n", lNuSY[f]);
//fflush(stdout);
//}
				}
			//}


			double ycontrib[NFEATURES];
			ZERO(ycontrib);

			// For all rated movies
			double bdampen = d0 / 1.4;
			double RuS = 1.0 / sqrt(d0);
			double yfactor = NuS/d0;
			for(j=0;j<d0;j++) {
				int m=userent[base0+j]&USER_MOVIEMASK;

				// Compute (ruj - buj) * wij and sumC
				//double rbw[NFEATURES];
			    //ZERO(rbj);
			    double rbw = 0.0;
			    double sumC = 0.0;
				for(j2=0;j2<d0;j2++) {
					int mm=userent[base0+j2]&USER_MOVIEMASK;
					if (m != mm) {
						double ee=err[base0+j2];
						double rb=(ee - bU[mm]);
						int loc = getOffset(m,mm);
						rbw += rb*wIJ[loc];
						//for(f=0;f<NFEATURES;f++)
						//rbw[f]+= rb*
						sumC += cIJ[loc];
					}
				}


				// Figure out the current error
				double ee=err[base0+j];
				double e2 = ee;
				e2 -= (bU[u] + bV[m]);
				e2 -= RuS * rbw;
				e2 -= NuS * sumC;
				for (f=0; f<NFEATURES; f++)
					e2 -= ((sU[u][f]+lNuSY[f])*sV[m][f]);

				// Train the biases
				double bUu = bU[u];
				double bVm = bV[m];
				bU[u] += Gamma0 * (e2 - bUu * L4) / bdampen;
				bV[m] += Gamma0 * (e2 - bVm * L4) / bdampen;
//printf("bU: %f\n", bU[u]);
//printf("bV: %f\n", bV[m]);
//fflush(stdout);

				// update U V and slope component of Y
				//double yfactor = NuS/sqrt(moviecount[m]); 
				//double yfactor = NuS;
				for (f=0; f<NFEATURES; f++) {
					double sUu = sU[u][f];
					double sVm = sV[m][f];

					sU[u][f] += (Gamma2 * ((e2 * sVm) - L8 * sUu));
					sV[m][f] += (Gamma2 * ((e2 * (sUu + lNuSY[f])) - L8 * sVm));
//printf("sU: %f\n", sU[u][f]);
//printf("sV: %f\n", sV[m][f]);
//fflush(stdout);

					ycontrib[f] += e2 * sVm * yfactor;
//printf("ycont: %f\n", ycontrib[f]);
//fflush(stdout);
				}

				// Update wij and cij
				for(j2=0;j2<d0;j2++) {
					int mm=userent[base0+j2]&USER_MOVIEMASK;
					if (m != mm) {
						double ee=err[base0+j2];
						double rb=(ee - bU[mm]);
						int loc = getOffset(m,mm);
						wIJ[loc] += Gamma2 * (RuS * e2 * rb - L8 * wIJ[loc]);
						cIJ[loc] += Gamma2 * (NuS * e2 - L8 * cIJ[loc]);
					}
				}
			}

			// Train Ys over all known movies for user
			for(j=0;j<dall;j++) {
				int m=userent[base0+j]&USER_MOVIEMASK;
				for (f=0; f<NFEATURES; f++) {
					double sYm = sY[m][f];
					sY[m][f] += Gamma2 * (ycontrib[f] - L7 * sYm);
//printf("before sY: %f\tycon: %f\tG2*ycon: %f\treg: %f\n", sY[m][f], ycontrib[f], (G2_Y*ycontrib[f]), G2_Y*L7_Y*sYm);
//printf("after sY: %f\tycon: %f\tG2*ycon: %f\treg: %f\n", sY[m][f], ycontrib[f], (G2_Y*ycontrib[f]), G2_Y*L7_Y*sYm);
//printf("sY: %f\tycon: %f\tG2*ycon: %f\n", sY[m][f], ycontrib[f], (G2*ycontrib[f]));
//fflush(stdout);
				}
			}
		}

		// Report rmse for main loop
		nrmse=0.;
		int ntrain=0;
		int elcnt=0;
		int k=2;
		int n=0;
		double s=0.;
		int i;
		for(u=0;u<NUSERS;u++) {
			int base0=useridx[u][0];
			int d0=UNTRAIN(u);
			int j,j2;

			// Setup the Ys again
			double sumY[NFEATURES];
			ZERO(sumY);
			double lNuSY[NFEATURES];
			ZERO(lNuSY);
			int dall=UNALL(u);
			double NuS = 1.0/sqrt(dall);
			double RuS = 1.0 / sqrt(d0);
			for(j=0;j<d0;j++) {
				int mm=userent[base0+j]&USER_MOVIEMASK;
				for(f=0;f<NFEATURES;f++)
					sumY[f]+=sY[mm][f];
			}
			for(j=0;j<d0;j++) {
				int mm=userent[base0+j]&USER_MOVIEMASK;
				for(f=0;f<NFEATURES;f++) 
					lNuSY[f] = NuS * sumY[f]; 
			}

			for(j=0;j<d0;j++) {
				int m=userent[base0+j]&USER_MOVIEMASK;
				double ee = err[base0+j];
				double e2 = ee;

				// Compute (ruj - buj) * wij and sumC
				//double rbw[NFEATURES];
			    //ZERO(rbj);
			    double rbw = 0.0;
			    double sumC = 0.0;
				for(j2=0;j2<d0;j2++) {
					int mm=userent[base0+j2]&USER_MOVIEMASK;
					if (m != mm) {
						double ee=err[base0+j2];
						double rb=(ee - bU[mm]);
						int loc = getOffset(m,mm);
						rbw += rb*wIJ[loc];
						//for(f=0;f<NFEATURES;f++)
						//rbw[f]+= rb*
						sumC += cIJ[loc];
					}
				}

				e2 -= (bU[u] + bV[m]);
				e2 -= RuS * rbw;
				e2 -= NuS * sumC;
				for (f=0; f<NFEATURES; f++)
					e2 -= ( (sU[u][f] + lNuSY[f]) * sV[m][f]);

if( elcnt++ == 5000 ) {
    printf("0 E: %f \t NE: %f\tNuSY: %f\tsV: %f\tsU: %f\tbU: %f\tbV: %f\tsY: %f\tU: %d\tM: %d\n", ee, e2, lNuSY[0], sV[m][0], sU[u][0], bU[u], bV[m], sY[m][0],u, m);
    printf("1 E: %f \t NE: %f\tNuSY: %f\tsV: %f\tsU: %f\tbU: %f\tbV: %f\tsY: %f\tU: %d\tM: %d\n", ee, e2, lNuSY[1], sV[m][1], sU[u][1], bU[u], bV[m], sY[m][1],u, m);
    printf("2 E: %f \t NE: %f\tNuSY: %f\tsV: %f\tsU: %f\tbU: %f\tbV: %f\tsY: %f\tU: %d\tM: %d\n", ee, e2, lNuSY[2], sV[m][2], sU[u][2], bU[u], bV[m], sY[m][2],u, m);
    printf("3 E: %f \t NE: %f\tNuSY: %f\tsV: %f\tsU: %f\tbU: %f\tbV: %f\tsY: %f\tU: %d\tM: %d\n", ee, e2, lNuSY[3], sV[m][3], sU[u][3], bU[u], bV[m], sY[m][3],u, m);
	fflush(stdout);
}
/*
if( e > 5.0 || e < -5.0 ) {
    printf("bad EE: %f\tU: %d\tM: %d\tNuSY: %f\te: %f\t sV: %f\tsU: %f\tbU: %f\tbV: %f\n", ee, u, m, NuSY, e, new_sV[m], new_sU[u], bUu, bVm);
	fflush(stdout);
}
*/

				nrmse+=e2*e2;
			}
			ntrain+=d0;

			int base=useridx[u][0];
			for(i=1;i<k;i++) base+=useridx[u][i];
			int d=useridx[u][k];
			for(i=0; i<d;i++) {
				int m=userent[base+i]&USER_MOVIEMASK;
				double rbw = 0.0;
				double sumC = 0.0;
				double RuS = 1.0/sqrt(d0);
				for(j2=0;j2<d0;j2++) {
					int mm=userent[base0+j2]&USER_MOVIEMASK;
					if (m != mm) {
						double ee=err[base0+j2];
						double rb=(ee - bU[mm]);
						int loc = getOffset(m,mm);
						rbw += rb*wIJ[loc];
						sumC += cIJ[loc];
					}
				}
				double e=err[base+i];
				e-=(bU[u] + bV[m]);
				e -= RuS * rbw;
				e -= NuS * sumC;
				for (f=0; f<NFEATURES; f++)
					e-=((sU[u][f] + lNuSY[f]) * sV[m][f]);
				s+=e*e;
			}
			n+=d;
		}
		nrmse=sqrt(nrmse/ntrain);
		//double prmse = rmseprobe();
		double prmse = sqrt(s/n);
		
		lg("%f\t%f\t%f\n",nrmse,prmse,(clock()-t0)/(double)CLOCKS_PER_SEC);
		//rmse_print(0);
		if ( loopcount < 14 ) {
		    Gamma2 *= 0.98;
		    Gamma0 *= 0.98;
		} else {
		    Gamma2 *= 0.92;
		    Gamma0 *= 0.92;
		}
	}
	
	/* Perform a final iteration in which the errors are clipped and stored */
	removeUV();
	
	//if(save_model) {
		//dappend_bin(fnameV,sV,NMOVIES);
		//dappend_bin(fnameU,sU,NUSERS);
	//}
	
	return 1;
}
