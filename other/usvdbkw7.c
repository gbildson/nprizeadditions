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

int    highM[NMOVIES];
int    lowM[NMOVIES];
	
FILE *fpR=NULL;
FILE *fpV=NULL, *fpU=NULL;
int moviecount[NMOVIES];
void score_setup()
{
	char *fname_rmovie = "rated_high.txt";
	if(fname_rmovie != NULL) {
	    //printf("Got Here\n");
		//fflush(stdout);
		int m;
		for(m=0;m<NMOVIES;m++) 
			highM[m] = 0;
		fpR=fopen(fname_rmovie,"r");
		char line[200];
        while (fgets(line, 200, fpR) != NULL) {
            m = atoi(line);
			//printf("high m = %d\n", m);
			highM[m] = 1;
        }
		fclose(fpR);
	    //printf("Done Here\n");
		//fflush(stdout);
	}

	fname_rmovie = "rated_low.txt";
	if(fname_rmovie != NULL) {
	    //printf("Got Here\n");
		//fflush(stdout);
		int m;
		for(m=0;m<NMOVIES;m++) 
			lowM[m] = 0;
		fpR=fopen(fname_rmovie,"r");
		char line[200];
        while (fgets(line, 200, fpR) != NULL) {
            m = atoi(line);
			//printf("low m = %d\n", m);
			lowM[m] = 1;
        }
		fclose(fpR);
	    //printf("Done Here\n");
		//fflush(stdout);
	}
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
double hsU[NUSERS],lsU[NUSERS];
void removeUV()
{
	int u;
	for(u=0;u<NUSERS;u++) {
		int base0=useridx[u][0];
		int d012=UNALL(u);
		int i;
		for(i=0; i<d012;i++) {
			int m=userent[base0+i]&USER_MOVIEMASK;
			double sUu;
			if ( highM[m] == 1 )
				sUu = hsU[u];
			else if ( lowM[m] == 1 )
				sUu = lsU[u];
			else 
				sUu = sU[u];
			err[base0+i]-=sUu*sV[m];
		}
	}
}

double new_sV[NMOVIES], new_sVV[NMOVIES];
double new_sU[NUSERS];
double new_hsU[NUSERS];
double new_lsU[NUSERS];
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
			new_hsU[u]=0.1;
			new_lsU[u]=0.1;
		}
		for(m=0;m<NMOVIES;m++) {
			new_sV[m]=0.1;
		}
	}
	
	/* Optimize current feature */
	double rmse=2., last_rmse=10.;
	double thr=sqrt(1.-E);
	while(rmse/last_rmse<thr) {
		int m;
		for(m=0;m<NMOVIES;m++) sV[m]=new_sV[m];
		int u;
		for(u=0;u<NUSERS;u++) {
			sU[u]=new_sU[u];
			hsU[u]=new_hsU[u];
			lsU[u]=new_lsU[u];
		}
		
		last_rmse=rmse;
		clock_t t0=clock();
		ZERO(new_sV);
		ZERO(new_sVV);
		double asVm=0;
		for(u=0;u<NUSERS;u++) {
			int base0=useridx[u][0];
			int d0=UNTRAIN(u);
			int j;
			double e[NMOVIES];
			double w[NMOVIES];
			int m[NMOVIES];
			for(j=0;j<d0;j++) {
				int mm=userent[base0+j]&USER_MOVIEMASK;
				m[j]=mm;
				int nuv=moviecount[mm];
				if(d0<nuv) nuv=d0;
				w[j]=wgt[base0+j];
				e[j]=w[j]*err[base0+j]*nuv/(nuv+ALPHA*nfeatures);
				asVm+=sV[m[j]];
			}
			asVm /= d0;
			
			double t_new_sU=0., new_sUU=0.;
			double t_new_hsU=0., new_hsUU=0.;
			double t_new_lsU=0., new_lsUU=0.;
			double eterm, wterm;
			double geterm=0.0, gwterm=0.0;
			for(j=0;j<d0;j++) {
				double sVm=sV[m[j]]; // + 0.10*asVm;
				int mm=m[j];
				eterm=(0.3333-0.05)*e[j]*sVm;
				wterm=(0.3333-0.05)*w[j]*sVm*sVm;
				//geterm+=0.15*e[j]*sVm;
				//gwterm+=0.15*w[j]*sVm*sVm;
				if ( highM[mm] ) {
				    t_new_hsU+=eterm;
				    new_hsUU+=wterm;
				    t_new_lsU+=eterm;
				    new_lsUU+=wterm;
				    t_new_sU+=eterm;
				    new_sUU+=wterm;
				} else if ( lowM[mm] ) {
				    t_new_hsU+=eterm;
				    new_hsUU+=wterm;
				    t_new_lsU+=eterm;
				    new_lsUU+=wterm;
				    t_new_sU+=eterm;
				    new_sUU+=wterm;
				} else {
				    t_new_hsU+=eterm;
				    new_hsUU+=wterm;
				    t_new_lsU+=eterm;
				    new_lsUU+=wterm;
				    t_new_sU+=eterm;
				    new_sUU+=wterm;
				}
			}
			
			//geterm/=gwterm;
			if ( new_sUU != 0 )
			    t_new_sU/=new_sUU;
			if ( new_hsUU != 0 )
				t_new_hsU/=new_hsUU;
			if ( new_lsUU != 0 )
				t_new_lsU/=new_lsUU;
			new_sU[u]=t_new_sU;
			new_hsU[u]=t_new_hsU;
			new_lsU[u]=t_new_lsU;

			double sUusUu=t_new_sU*t_new_sU;
			//double gsUusUu=geterm*geterm;
			double hsUusUu=t_new_hsU*t_new_hsU;
			double lsUusUu=t_new_lsU*t_new_lsU;
			double geadd, gwadd;
			for(j=0;j<d0;j++) {
				int mm=m[j];
				//geadd = e[j]*geterm;
				//gwadd = w[j]*gsUusUu;
				if ( highM[mm] ) {
				    new_sV[mm]+=e[j]*t_new_hsU;
				    new_sVV[mm]+=w[j]*hsUusUu;
				} else if ( lowM[mm] ) {
				    new_sV[mm]+=e[j]*t_new_lsU;
				    new_sVV[mm]+=w[j]*lsUusUu;
				} else {
				    new_sV[mm]+=e[j]*t_new_sU;
				    new_sVV[mm]+=w[j]*sUusUu;
				}
			}
		    new_sV[1] = 1.0;
		    sV[1] = 1.0;
			sU[0]=1.0;
			hsU[0]=1.0;
			lsU[0]=1.0;
			new_sU[0]=1.0;
			new_hsU[0]=1.0;
			new_lsU[0]=1.0;
		}
		for(m=0;m<NMOVIES;m++) new_sV[m]/=new_sVV[m];

		rmse=0.;
		int ntrain=0;
		for(u=0;u<NUSERS;u++) {
			int base0=useridx[u][0];
			int d0=UNTRAIN(u);
			int j;
			for(j=0;j<d0;j++) {
				int m=userent[base0+j]&USER_MOVIEMASK;
				double e = err[base0+j]-new_sU[u]*new_sV[m];
				if ( highM[m] ) 
					e = err[base0+j]-new_hsU[u]*new_sV[m];
				else if ( lowM[m] )
					e = err[base0+j]-new_lsU[u]*new_sV[m];
				else
					e = err[base0+j]-new_sU[u]*new_sV[m];
				rmse+=e*e;
				//if (e < 0.0 )
					//e = -e;
				//rmse+=e;
			}
			ntrain+=d0;
		}
		rmse=sqrt(rmse/ntrain);
		
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
