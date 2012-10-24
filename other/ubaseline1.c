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
* Perform score tests using user data
*/
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "basic.h"
#include "netflix.h"
#include "utest.h"
/*
* Additional base line score from:
* Improved Neighborhood-based Collaborative Filtering
* Robert M. Bell and Yehuda Koren
* Neighbor-Koren.pdf
*
* The alpha values were taken from YehudaKoren post at
* http://www.netflixprize.com/community/viewtopic.php?id=772
*/
double movieavg[NMOVIES];
int moviecount[NMOVIES];
double useravg[NUSERS];
int usercount[NUSERS];
int day0[NUSERS];


// The ALPHA value (25.) also appears at http://sifter.org/~simon/journal/20061211.html
#define MOVIEAVG_ALPHA	(25.)


void mavg()
{
	lg("Movie avg centering\n");
	ZERO(movieavg);
	ZERO(moviecount);
	int u;
	for(u=0;u<NUSERS;u++) {
		int base=useridx[u][0];
		int d=UNTRAIN(u);
		int i;
		for(i=0; i<d;i++) {
			int m=userent[base+i]&USER_MOVIEMASK;
			movieavg[m]+=err[base+i];
			moviecount[m]++;
		}
	}
	int m;
	for(m=0;m<NMOVIES;m++)
		movieavg[m]/=moviecount[m]+MOVIEAVG_ALPHA;
	int i;
	for(i=0;i<NENTRIES;i++) {
		int m=userent[i]&USER_MOVIEMASK;
		err[i]-=movieavg[m];
	}
}

#define USERAVG_ALPHA	(7.)

void uavg()
{
	lg("User avg centering\n");
	ZERO(useravg);
	int u;
	for(u=0;u<NUSERS;u++) {
		int base=useridx[u][0];
		int d0=UNTRAIN(u);
		int i;
		for(i=0; i<d0;i++)
			useravg[u]+=err[base+i];
		useravg[u]/=d0+USERAVG_ALPHA;
	}
	for(u=0;u<NUSERS;u++) {
		int base=useridx[u][0];
		int d012=UNALL(u);
		int i;
		for(i=0; i<d012;i++) {
			err[base++]-=useravg[u];
		}
	}
}

void userXX(double *usertime,float *ein, int d0, int d012, double alpha)
{
	// Remove average but only use training data because we can only correlate it with ein
	double avg=dvavg(usertime,d0);
	int j;
	for(j=0;j<d012;j++) usertime[j]-=avg;
	// compute unbiased estimator
	double theta=fdvdot(ein,usertime,d0)/(dvsqr(usertime,d0)+1.e-20);
	theta*=d0/(d0+alpha); // scale it
	// Make prediction
	for(j=0;j<d012;j++)
		ein[j]-=theta*usertime[j];
}
#define DTIME(t) (sqrt((t)&0xfffffff0))
#define USERTIME_ALPHA (550.)
void usertime()
{
	lg("User Time\n");
	int u;
	for(u=0;u<NUSERS;u++) {
		int base=useridx[u][0];
		int d012=UNALL(u);
		int d0=UNTRAIN(u);
		// It is OK to look on all data for day0 because it is always known
		int day0=uivmin(&userent[base],UNTOTAL(u))>>(USER_LMOVIEMASK+3);
		// compute explanatory variable
		double usertime[NMOVIES];
		int j;
		for(j=0;j<d012;j++) {
			int day=userent[base+j]>>(USER_LMOVIEMASK+3);
			usertime[j]=DTIME(day-day0);
		}
		
		userXX(usertime,&err[base],d0,d012,USERTIME_ALPHA);
	}
}

#define USERTIMEMOVIE_ALPHA (150.)
void usertimemovie()
{
	lg("User Time(Movie)\n");
	int day0[NMOVIES];
	ZERO(day0);
	// It is OK to look on all data for day0 because it is always known
	int i;
	for(i=0;i<NENTRIES;i++) {
		int m=userent[i]&USER_MOVIEMASK;
		int day=userent[i]>>(USER_LMOVIEMASK+3);
		if(!day0[m] || day0[m]>day) day0[m]=day;
	}
	
	int u;
	for(u=0;u<NUSERS;u++) {
		int base=useridx[u][0];
		int d012=UNALL(u);
		int d0=UNTRAIN(u);
		// compute explanatory variable
		double usertime[NMOVIES];
		int j;
		for(j=0;j<d012;j++) {
			int m=userent[base+j]&USER_MOVIEMASK;	
			int day=userent[base+j]>>(USER_LMOVIEMASK+3);
			usertime[j]=DTIME(day-day0[m]);
		}
		userXX(usertime,&err[base],d0,d012,USERTIMEMOVIE_ALPHA);
	}
}

#define MOVIETIME_ALPHA (4000.)
void movietime()
{
	lg("Movie Time\n");
	int day0[NMOVIES];
	ZERO(day0);
	// It is OK to look on all data for day0 because it is always known
	int i;
	for(i=0;i<NENTRIES;i++) {
		int m=userent[i]&USER_MOVIEMASK;
		int day=userent[i]>>(USER_LMOVIEMASK+3);
		if(!day0[m] || day0[m]>day) day0[m]=day;
	}
	
	// Remove average but only use training data
	double avg[NMOVIES];
	int moviecount[NMOVIES];
	ZERO(avg);
	ZERO(moviecount);
	int u;
	for(u=0;u<NUSERS;u++) {
		int base=useridx[u][0];
		int d0=UNTRAIN(u);
		int j;
		for(j=0;j<d0;j++) {
			int m=userent[base+j]&USER_MOVIEMASK;
			int day=userent[base+j]>>(USER_LMOVIEMASK+3);
			avg[m]+=DTIME(day-day0[m]);
			moviecount[m]++;
		}
	}
	int m;
	for(m=0;m<NMOVIES;m++) avg[m]/=moviecount[m];

	// compute unbiased estimator
	double theta[NMOVIES];
	double var[NMOVIES];
	ZERO(theta);
	ZERO(var);
	for(u=0;u<NUSERS;u++) {
		int base=useridx[u][0];
		int d0=UNTRAIN(u);
		int j;
		for(j=0;j<d0;j++) {
			int m=userent[base+j]&USER_MOVIEMASK;
			int day=userent[base+j]>>(USER_LMOVIEMASK+3);
			// compute explanatory variable
			double x=DTIME(day-day0[m])-avg[m];
			theta[m]+=err[base+j]*x;
			var[m]+=x*x;
		}
	}
	for(m=0; m<NMOVIES; m++)
		theta[m]=(theta[m]/(var[m]+1.e-20))*moviecount[m]/(moviecount[m]+MOVIETIME_ALPHA);
	
	//predict
	for(i=0;i<NENTRIES;i++) {
		int m=userent[i]&USER_MOVIEMASK;
		int day=userent[i]>>(USER_LMOVIEMASK+3);
		double x=DTIME(day-day0[m])-avg[m];
		err[i]-=theta[m]*x;
	}
}

#define MOVIETIMEUSER_ALPHA (500.)

void movietimeuser()
{
	lg("Movie Time(User)\n");
	ZERO(day0);
	// It is OK to look on all data for day0 because it is always known
	int u;
	for(u=0;u<NUSERS;u++) {
		int base=useridx[u][0];
		int d012=UNTOTAL(u);
		int j;
		for(j=0;j<d012;j++) {
			int m=userent[base+j]&USER_MOVIEMASK;
			int day=userent[base+j]>>(USER_LMOVIEMASK+3);
			if(!day0[u] || day0[u]>day) day0[u]=day;
		}
	}
	
	// Remove average but only use training data
	double avg[NMOVIES];
	int moviecount[NMOVIES];
	ZERO(avg);
	ZERO(moviecount);
	for(u=0;u<NUSERS;u++) {
		int base=useridx[u][0];
		int d0=UNTRAIN(u);
		int j;
		for(j=0;j<d0;j++) {
			int m=userent[base+j]&USER_MOVIEMASK;
			int day=userent[base+j]>>(USER_LMOVIEMASK+3);
			avg[m]+=DTIME(day-day0[u]);
			moviecount[m]++;
		}
	}
	int m;
	for(m=0;m<NMOVIES;m++) avg[m]/=moviecount[m];

	// compute unbiased estimator
	double theta[NMOVIES];
	double var[NMOVIES];
	ZERO(theta);
	ZERO(var);
	for(u=0;u<NUSERS;u++) {
		int base=useridx[u][0];
		int d0=UNTRAIN(u);
		int j;
		for(j=0;j<d0;j++) {
			int m=userent[base+j]&USER_MOVIEMASK;
			int day=userent[base+j]>>(USER_LMOVIEMASK+3);
			// compute explanatory variable
			double x=DTIME(day-day0[u])-avg[m];
			theta[m]+=err[base+j]*x;
			var[m]+=x*x;
		}
	}
	for(m=0; m<NMOVIES; m++)
		theta[m]=(theta[m]/(var[m]+1.e-20))*moviecount[m]/(moviecount[m]+MOVIETIME_ALPHA);
	
	//predict
	for(u=0;u<NUSERS;u++) {
		int base=useridx[u][0];
		int d012=UNALL(u);
		int j;
		for(j=0;j<d012;j++) {
			int m=userent[base+j]&USER_MOVIEMASK;
			int day=userent[base+j]>>(USER_LMOVIEMASK+3);
			double x=DTIME(day-day0[u])-avg[m];
			err[base+j]-=theta[m]*x;
		}
	}
}

void userXmovie(double *xmovie, double alpha)
{
	int u;
	for(u=0;u<NUSERS;u++) {
		int base=useridx[u][0];
		int d012=UNALL(u);
		int d0=UNTRAIN(u);
		// compute avg explanatory variable
		double x[NMOVIES];
		int j;
		for(j=0;j<d012;j++) {
			int m=userent[base+j]&USER_MOVIEMASK;	
			x[j]=xmovie[m];
		}
		
		userXX(x,&err[base],d0,d012,alpha);
	}
}

#define USERAVGMOVIE_ALPHA (90.)
void useravgmovie()
{
	lg("User AvgMovie\n");
	double avgmovie[NMOVIES];
	int moviecount[NMOVIES];
	ZERO(avgmovie);
	ZERO(moviecount);
	int u;
	for(u=0;u<NUSERS;u++) {
		int base=useridx[u][0];
		int d0=UNTRAIN(u);
		int j;
		for(j=0;j<d0;j++) {
			int m=userent[base+j]&USER_MOVIEMASK;
			int r=(userent[base+j]>>USER_LMOVIEMASK)&7;
			avgmovie[m]+=r;
			moviecount[m]++;
		}
	}
	int m;
	for(m=0;m<NMOVIES;m++) avgmovie[m]/=moviecount[m];
	
	userXmovie(avgmovie,USERAVGMOVIE_ALPHA);
}

#define USERCNTMOVIE_ALPHA (90.)
void usercntmovie()
{
	lg("User Movie Count\n");
	double avgmovie[NMOVIES];
	ZERO(avgmovie);
	// It is OK to look on all data for day0 because it is always known
	int u;
	for(u=0;u<NUSERS;u++) {
		int base=useridx[u][0];
		int d012=UNTOTAL(u);
		int j;
		for(j=0;j<d012;j++) {
			int m=userent[base+j]&USER_MOVIEMASK;
			avgmovie[m]+=1.;
		}
	}
	
	userXmovie(avgmovie,USERCNTMOVIE_ALPHA);
}

void movieXuser(double *xuser, double alpha)
{
	// Remove average but only use training data
	double avg[NMOVIES];
	int moviecount[NMOVIES];
	ZERO(avg);
	ZERO(moviecount);
	int u;
	for(u=0;u<NUSERS;u++) {
		int base=useridx[u][0];
		int d0=UNTRAIN(u);
		int j;
		double xu=xuser[u];
		for(j=0;j<d0;j++) {
			int m=userent[base+j]&USER_MOVIEMASK;
			avg[m]+=xu;
			moviecount[m]++;
		}
	}
	int m;
	for(m=0;m<NMOVIES;m++) avg[m]/=moviecount[m];

	// compute unbiased estimator
	double theta[NMOVIES];
	double var[NMOVIES];
	ZERO(theta);
	ZERO(var);
	for(u=0;u<NUSERS;u++) {
		int base=useridx[u][0];
		int d0=UNTRAIN(u);
		int j;
		double xu=xuser[u];
		for(j=0;j<d0;j++) {
			int m=userent[base+j]&USER_MOVIEMASK;
			// compute explanatory variable
			double x=xu-avg[m];
			theta[m]+=err[base+j]*x;
			var[m]+=x*x;
		}
	}
	for(m=0; m<NMOVIES; m++)
		theta[m]=(theta[m]/(var[m]+1.e-20))*moviecount[m]/(moviecount[m]+alpha);
	
	//predict
	for(u=0;u<NUSERS;u++) {
		int base=useridx[u][0];
		int d012=UNALL(u);
		int j;
		double xu=xuser[u];
		for(j=0;j<d012;j++) {
			int m=userent[base+j]&USER_MOVIEMASK;
			double x=xu-avg[m];
			err[base+j]-=theta[m]*x;
		}
	}
}

#define MOVIEAVGUSER_ALPHA (50.)
void movieavguser()
{
	lg("Movie Avg User\n");
	int u;
	// Remove average but only use training data
	double avg[NMOVIES];
	int moviecount[NMOVIES];
	ZERO(avg);
	ZERO(moviecount);
	for(u=0;u<NUSERS;u++) {
		int base=useridx[u][0];
		int d0=UNTRAIN(u);
		int j;
		for(j=0;j<d0;j++) {
			int m=userent[base+j]&USER_MOVIEMASK;
			int r=(userent[base+j]>>USER_LMOVIEMASK)&7;
			avg[m]+=r;
			moviecount[m]++;
		}
	}
	int m;
	for(m=0;m<NMOVIES;m++) avg[m]/=moviecount[m];
	
	for(u=0;u<NUSERS;u++) {
		int base=useridx[u][0];
		int d0=UNTRAIN(u);
		int j;
		double sum=0.;
		for(j=0;j<d0;j++) {
			int m=userent[base+j]&USER_MOVIEMASK;
			int r=(userent[base+j]>>USER_LMOVIEMASK)&7;
			sum+=r-avg[m];
		}
		useravg[u]=sum/d0;
	}
	
	movieXuser(useravg,MOVIEAVGUSER_ALPHA);
}

#define MOVIECNTUSER_ALPHA (50.)
void moviecntuser()
{
	lg("Movie Cnt User\n");
	int u;
	// It is OK to look on all data for day0 because it is always known
	for(u=0;u<NUSERS;u++) {
		int d0=UNTRAIN(u);
		int d012=UNTOTAL(u);
		useravg[u]=d012;
	}
	
	movieXuser(useravg,MOVIECNTUSER_ALPHA);
}

#define TIMECORR2_ALPHA (180.)
void timecorr2()
{	
	lg("Time corr 2\n");
	int u;	
	for(u=0;u<NUSERS;u++) {
		int base0=useridx[u][0];
		int d012=UNTOTAL(u);
		int daycount[MAX_DAY+1];
		ZERO(daycount);
		int i;
		for(i=0; i<d012;i++) {
			int day=userent[base0+i]>>USER_LDAY;
			daycount[day]++;
		}
			
		int d0=UNTRAIN(u);
		double esum=0., var=0.;
		double avg=0., corr=0.;
		for(i=0; i<d0;i++) {
			int day=userent[base0+i]>>USER_LDAY;
			double e=err[base0+i];
			double x=daycount[day];
			esum+=e;
			avg+=x;
			var+=x*x;
			corr+=x*e;
		}
		avg/=d0;
		var=var/d0-avg*avg;
		corr=corr/d0-avg*esum/d0;
		corr/=(var+EPS);
		corr*=d0/(d0+TIMECORR2_ALPHA);
	
		for(i=0; i<d012;i++) {
			int day=userent[base0+i]>>USER_LDAY;
			double x=daycount[day];
			err[base0+i]-=corr*(x-avg);
		}
	}
}

int nmethods=0;
int methods[20];
int score_argv(char **argv)
{
	if(strcmp(argv[0],"-bl")) return 0;
	methods[nmethods++]=atoi(argv[1]);
	return 2;
}

void score_setup()
{
	if(!nmethods) {
		int defaultmethods[]={0,1,2,6,7,10,8,3,4,5,9,0};
		nmethods=sizeof(defaultmethods)/sizeof(int);
		memcpy(methods,defaultmethods,sizeof(defaultmethods));
	}
}


int score_train(int loop) {
	int method=loop;
	if(nmethods) {
		if(loop>=nmethods) return 0;
		method=methods[loop];
	}
	switch(method) {
	case 0:
		mavg();
		break;
	case 1:
		uavg();
		break;
	case 2:
		usertime();
		break;
	case 3:
		usertimemovie();
		break;
	case 4:
		movietime();
		break;
	case 5:
		movietimeuser();
		break;
	case 6:
		useravgmovie();
		break;
	case 7:
		usercntmovie();
		break;
	case 8:
		movieavguser();
		break;
	case 9:
		moviecntuser();
		break;
	case 10:
		timecorr2();
		break;
	case 11:
		globalavg();
		break;
	default:
		return 0;
	}
	return 1;
}
