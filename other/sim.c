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
* Perform score tests using data generated with moviebin2userbin and qualify2bin.py
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "basic.h"
#include "netflix.h"
#include "utest.h"

int aopt=0;
int load_model=0;
int save_model=0;
int dontclip=0;
char *fname_outerr=NULL;
char *useridx_path="data/user_index.bin";
char *userent_path="data/user_entry.bin";

#define QQPP (1442271)

int useridx[NUSERS][4];
unsigned int userent[NENTRIES];
//float err[NENTRIES];

//struct {
	//int n;
	//float delta_x;
	//float delta_y;
	//float sum_sq_x;
	//float sum_sq_y;
	//float sum_coproduct;
	//float mean_x;
	//float mean_y;
//};
//pear[157895334];
//pear[10]; 
float dx[157895334];
float dy[157895334];
float sx[157895334];
float sy[157895334];
float sc[157895334];
float mx[157895334];
float my[157895334];
int   n[157895334];

//PWORK pear[157895334];

void clip(float *ein, unsigned int *uent, float *eout, int d)
{
	int i;
	for(i=0; i<d; i++) {
		float e=ein[i];
		int r=(uent[i]>>USER_LMOVIEMASK)&7;
		e=r-e; // extract prediction from real results and error
		if(e>4.) e=4.;
		else if(e<0.) e=0.;
		eout[i]=r-e; // convert prediction back to error
	}
}

void cliperr()
{
	lg("Clipping errors\n");
	int u;
	for(u=0;u<NUSERS;u++) {
		int base=useridx[u][0];
		int d012=UNTOTAL(u); // no sense in not clipping the qualifing results
		//clip(&err[base],&userent[base],&err[base],d012);
	}
}

double cliprmse(int k)
{
	int u;
	int n=0;
	double s=0.;
	int i;
		
	if(k==3) {
		for(u=0;u<NUSERS;u++) {
			int base=useridx[u][0];
			int d=useridx[u][1]+useridx[u][2];
			float etmp[NMOVIES];
			//clip(&err[base],&userent[base],etmp,d);		
			s+=fvsqr(etmp,d);
			n+=d;
		}
	} else {
		for(u=0;u<NUSERS;u++) {
			int base=useridx[u][0];
			for(i=1;i<k;i++) base+=useridx[u][i];
			int d=useridx[u][k];
			float etmp[NMOVIES];
			//clip(&err[base],&userent[base],etmp,d);		
			s+=fvsqr(etmp,d);
			n+=d;
		}
	}
	return sqrt(s/n);
}

double rmse(int k)
{
	int u;
	int n=0;
	double s=0.;
	int i;

	if(k==3) {
		for(u=0;u<NUSERS;u++) {
			int base=useridx[u][0];
			int d=useridx[u][1]+useridx[u][2];
			//s+=fvsqr(&err[base],d);
			n+=d;
		}
	} else {
		for(u=0;u<NUSERS;u++) {
			int base=useridx[u][0];
			for(i=1;i<k;i++) base+=useridx[u][i];
			int d=useridx[u][k];
			//s+=fvsqr(&err[base],d);
			n+=d;
		}
	}
	return sqrt(s/n);
}

double last_rmse_train=-1,last_rmse_train_clipped=-1;
double last_rmse_probe=-1,last_rmse_probe_clipped=-1;
double last_rmse_both=-1,last_rmse_both_clipped=-1;
void rmse_print(int copt)
{
	double rmse_train=rmse(1);
	double rmse_probe=rmse(2);
	double rmse_both=rmse(3);
	if(!copt) {
		double rmse_train_clipped=cliprmse(1);
		double rmse_probe_clipped=cliprmse(2);
		double rmse_both_clipped=cliprmse(3);
		lg("RMSE Train %f (%.1f%%) Clipped %f (%.1f%%) Probe %f (%.1f%%) Clipped %f Both %f (%.1f%%) Clipped %f\n",
			rmse_train,100.*(last_rmse_train-rmse_train)/rmse_train,
			rmse_train_clipped,100.*(last_rmse_train_clipped-rmse_train_clipped)/rmse_train_clipped,
			rmse_probe,100.*(last_rmse_probe-rmse_probe)/rmse_probe,
			rmse_probe_clipped,100.*(last_rmse_probe_clipped-rmse_probe_clipped)/rmse_probe_clipped,
			rmse_both_clipped,100.*(last_rmse_both_clipped-rmse_both_clipped)/rmse_both_clipped
			);
		last_rmse_probe_clipped=rmse_probe_clipped;
		last_rmse_train_clipped=rmse_train_clipped;
	} else
		lg("RMSE Train %f (%.1f%%) Probe %f (%.1f%%) Both %f (%.1f%%)\n",
			rmse_train,100.*(last_rmse_train-rmse_train)/rmse_train,
			rmse_probe,100.*(last_rmse_probe-rmse_probe)/rmse_probe,
			rmse_both,100.*(last_rmse_both-rmse_both)/rmse_both
		);
	last_rmse_both=rmse_both;
	last_rmse_probe=rmse_probe;
	last_rmse_train=rmse_train;
}

int getOffset(int i, int j) {
    return i * (NMOVIES) - (i+1) * i / 2 + j - 1;
}

main(int argc, char**argv) {
//printf("got here\n");
//}
//foobar(int argc, char**argv) {
	lgopen(argc,argv);
	char *fname_inerr[5];
	int nscores=0;
	int nweights=0;
	double weights[100];
	char *fname_qualify=NULL;
	int nloops=10000;
	int copt=1;
	int i;
	for(i=1;i<argc;i++) {
		int rc=score_argv(argv+i);
		if(rc>0) {
			i+=rc-1;
			continue;
		}
		if(!strcmp(argv[i],"-le"))
			fname_inerr[nscores++]=argv[++i];
		else if(!strcmp(argv[i],"-lew"))
			weights[nweights++]=atof(argv[++i]);
		else if(!strcmp(argv[i],"-se"))
			fname_outerr=argv[++i];
		else if(!strcmp(argv[i],"-sq"))
			fname_qualify=argv[++i];
		else if(!strcmp(argv[i],"-l"))
			nloops=atoi(argv[++i]);
		else if(!strcmp(argv[i],"-c"))
			copt^=1;
		else if(!strcmp(argv[i],"-a"))
			aopt=1;
		else if(!strcmp(argv[i],"-lm"))
			load_model=1;
		else if(!strcmp(argv[i],"-sm"))
			save_model=1;
		else {
			lg("Unrecognized argument %d %s ?\n",i,argv[i]);
			lg("-le <fname> - load precomputed error file.\n");
			lg("-lew <weight> - In case of several -le, use wrights, instead of fit\n");
			lg("-se <fname> - store resulted error file.\n");
			lg("-l <n> - number of training loops to perform\n");
			lg("-a - Perform training also on probe data\n");
			lg("-c - Dont clip scores to be between 0...4\n");
			lg("-sq <fname> - write qualifying submission to file.\n");
			lg("-lm - load precomputed model.\n");
			lg("-sm - save computed model.\n");
			exit(0);
		}
	}
	if(fname_qualify && !aopt)
		lg("WARNING: -sq without -a\n");
	if(fname_qualify && !copt)
		lg("WARNING: -sq with -c\n");
	if(nweights && nscores && nweights!=nscores)
		lg("Number of weights %d (-lew) does not match number of files %d (-le)\n",nweights,nscores);
	
	load_bin(useridx_path,useridx,sizeof(useridx));
	{
		int count[4],u,k;
		ZERO(count);
		for(u=0;u<NUSERS;u++)
			for(k=1;k<4;k++)
				count[k]+=useridx[u][k];
		lg("Train=%d Probe=%d Qualify=%d\n",count[1],count[2],count[3]);
	}	
	load_bin(userent_path,userent,sizeof(userent));

	int m, u;
    unsigned int *movieent[NMOVIES];
    unsigned int moviecnt[NMOVIES];
    unsigned int moviealoc[NMOVIES];
	for(m=0;m<NMOVIES;m++) {
		movieent[m] = malloc(255*4);
		moviecnt[m] = 0;
		moviealoc[m] = 255;
	}

    for(u=0;u<NUSERS;u++) {
		int base=useridx[u][0];
		int d=UNTRAIN(u);
		int i;
		
		for(i=0; i<d;i++) {
			int m=userent[base+i]&USER_MOVIEMASK;
		    int r=(userent[base+i]>>USER_LMOVIEMASK)&7;
			unsigned int nent = u | (r<<MOVIE_LUSERMASK); 
			int mloc  = moviecnt[m]++;
			if ( moviealoc[m] <= mloc ) {
			    int nmloc = mloc * 2;
				if ( nmloc > 100000) {
					nmloc = mloc + 10000;
				}
    			unsigned int *omovieent = movieent[m];
		        movieent[m] = malloc(nmloc*4);
				int j;
				for (j=0; j < mloc; j++)
					movieent[m][j] = omovieent[j];
		        moviealoc[m] = nmloc;
				free(omovieent);
			}
			movieent[m][mloc] = nent;
		}
	}

	int maxoff=getOffset(NMOVIES-1,NMOVIES);
	printf("MAX OFF: %d   NMOVIES^2: %d\n", maxoff, NMOVIES*NMOVIES);
	fflush(stdout);
	int m1, m2;
	int loc;
	for(m1=0;m1<NMOVIES;m1++) {
	    for(m2=m1+1;m2<NMOVIES;m2++) {
	        loc=getOffset(m1,m2);

			if ( loc >= 157895334 ) {
	            printf("Too big: %d\n", loc);
	            fflush(stdout);
				break;
			}
	        //sum_sq_x = 0
			sx[loc] = 0.0f;

	        //sum_sq_y = 0
			sy[loc] = 0.0f;

	        //sum_coproduct = 0
			sc[loc] = 0.0f;
			
			n[loc] = 0;
		}
	}
	printf("Done\n");
	fflush(stdout);

	//mean_x = x[1]
	//mean_y = y[1]
	//for i in 2 to N:
		//sweep = (i - 1.0) / i
		//delta_x = x[i] - mean_x
		//delta_y = y[i] - mean_y
		//sum_sq_x += delta_x * delta_x * sweep
		//sum_sq_y += delta_y * delta_y * sweep
		//sum_coproduct += delta_x * delta_y * sweep
		//mean_x += delta_x / i
		//mean_y += delta_y / i 
	//pop_sd_x = sqrt( sum_sq_x/N )
	//pop_sd_y = sqrt( sum_sq_y/N )
	//cov_x_y = sum_coproduct/N
	//correlation = cov_x_y / (pop_sd_x * pop_sd_y)
}
