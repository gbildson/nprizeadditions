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
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>
#include "basic.h"

double drand48() {
	return ((double)rand())/((double)(RAND_MAX)+1.0);

}

long lrand48() {
	return (long)rand();

}

unsigned int uivmin(unsigned int *v, int n)
{
	unsigned int m=*v++;
	int i;
	for(i=1;i<n;i++) {
		unsigned int mm=*v++;
		if(mm<m) m=mm;
	}
	return m;
}

double dvavg(double *v, int n)
{
	double sum=0.;
	int i;
	for(i=0;i<n;i++)
		sum+=*v++;
	return sum/n;
}

double dvnorm(double *v, int n)
{
	double sum=0.;
	double *pv=v;
	int i;
	for(i=0;i<n;i++) sum+=*pv++;
	sum=1./sum;
	pv=v;
	for(i=0;i<n;i++) *pv++ *= sum;
}

void dvscale(double *v, double scale, int n)
{
	int i;
	for(i=0;i<n;i++) *v++ *= scale;
}

void dvadd(double *v1,double *v2,int n)
{
	int i;
	for(i=0; i<n; i++) *v1++ += *v2++;
}

double fdvdot(float *v1, double *v2, int n)
{
	int i;
	double sum=0.;
	for(i=0;i<n;i++) {
		sum+=(*v1++)*(*v2++);
	}
	return sum;
}


double ddvdot(double *v1, double *v2, int n)
{
	int i;
	double sum=0.;
	for(i=0;i<n;i++) {
		sum+=(*v1++)*(*v2++);
	}
	return sum;
}

double fdvwdot(float *v1, double *v2, int n,double *wgt)
{
	int i;
	double sum=0.;
	for(i=0;i<n;i++) {
		sum+=(*wgt++)*(*v1++)*(*v2++);
	}
	return sum;
}


double dvsqr(double *v, int n)
{
	int i;
	double sum=0.;
	for(i=0;i<n;i++) {
		double e=*v++;
		sum+=e*e;
	}
	return sum;
}

double dvwsqr(double *v, int n, double *wgt)
{
	int i;
	double sum=0.;
	for(i=0;i<n;i++) {
		double e=*v++;
		sum+=(*wgt++)*e*e;
	}
	return sum;
}

double fvsqr(float *v, int n)
{
	int i;
	double sum=0.;
	for(i=0;i<n;i++) {
		double e=*v++;
		sum+=e*e;
	}
	return sum;
}

FILE *lgfile=NULL;
void lg(char *fmt,...)
{
	char buf[2048];
	va_list ap;

	va_start(ap, fmt);
	vsprintf(buf,fmt,ap);
	va_end(ap);
	fprintf(stderr,"%s",buf);
	if(lgfile) {
		fprintf(lgfile,"%s",buf);
		fflush(lgfile);
	}
}

void error(char *fmt,...)
{
	char buf[2048];
	va_list ap;

	va_start(ap, fmt);
	vsprintf(buf,fmt,ap);
	va_end(ap);
	lg("%s",buf);
	lg("\n");
	exit(1);
}

void lgopen(int argc, char**argv)
{
	lgfile=fopen("data/log.txt","a");
	if(!lgfile) error("Cant open log file");
	lg("----------------------------------------------\n");
	/* Print out the date and time in the standard format.  */
	time_t curtime=time(NULL);
	lg("%s",ctime(&curtime));

	int i;
	for(i=0;i<argc;i++)
		lg("%s ",argv[i]);
	lg("\n");
}

void load_bin(char *path, void *data, int len)
{
    FILE *fp;
    lg("Loading %s\n",path);
    fp=fopen(path,"rb");
	if(!fp) {
		lg("Cant open file\n");
		exit(1);
	}
    if(len!=fread(data,1,len,fp)) {
        lg("Failed to read all data\n");
        exit(1);
    }
    fclose(fp);
}

void dump_bin(char *path, void *data, int len)
{
    FILE *fp;
    lg("Writing %s\n",path);
    fp=fopen(path,"wb");
    if(len!=fwrite(data,1,len,fp)) {
        lg("Failed to write all data\n");
        exit(1);
    }
    fclose(fp);
}

void ddump_bin(char *fname,double *vec,int M,int N,int N1)
{
	int m;
	FILE *fp;
	
	lg("Writing to %s\n",fname);
	fp=fopen(fname,"wb");
	for(m=0;m<M;m++) {
		if(N!=fwrite(vec,sizeof(double),N,fp)) {
			lg("Error\n");
			exit(1);
		}
		vec+=N1;
	}
	fclose(fp);
}

void dappend_bin(char *fname,double *vec,int N)
{
	int m;
	FILE *fp;
	
	lg("Appending to %s\n",fname);
	if(NULL==(fp=fopen(fname,"ab")))
		error("Cant append");
	if(N!=fwrite(vec,sizeof(double),N,fp))
		error("Cant write");
	fclose(fp);
}

int dload_bin(char *fname,double *vec,int M,int N1)
{
	int m,n,N,s;
	FILE *fp;
	
	lg("Loading %s\n",fname);
	
	fp=fopen(fname,"rb");
	if(!fp) {
		lg("Cant open file\n");
		return 0;
	}
	fseek(fp,0,SEEK_END);
	s=ftell(fp);
	fseek(fp,0,SEEK_SET);
	N=s/sizeof(double)/M;
	lg("N=%d\n",N);
	if(N*sizeof(double)*M != s) {
		lg("File size does not divide\n");
		exit(1);
	}
	if(N<1 || N>N1) {
		lg("Bad N %d\n",N1);
		exit(1);
	}
	
	for(m=0;m<M;m++) {
		n=fread(vec,sizeof(double),N,fp);
		if(N!=n) {
			lg("Error %d %d\n",m,n);
			exit(1);
		}
		vec+=N1;
	}
	fclose(fp);
	return N;
}


int monthdays[2][12]={
{0,31,31+28,31+28+31,31+28+31+30,31+28+31+30+31,31+28+31+30+31+30,
31+28+31+30+31+30+31,31+28+31+30+31+30+31+31,31+28+31+30+31+30+31+31+30,
31+28+31+30+31+30+31+31+30+31,31+28+31+30+31+30+31+31+30+31+30},
{0,31,31+29,31+29+31,31+29+31+30,31+29+31+30+31,31+29+31+30+31+30,
31+29+31+30+31+30+31,31+29+31+30+31+30+31+31,31+29+31+30+31+30+31+31+30,
31+29+31+30+31+30+31+31+30+31,31+29+31+30+31+30+31+31+30+31+30}};
int yeardays[]={0,365,365+365,365+365+366,365+365+366+365,365+365+366+365+365,365+365+366+365+365+365,365+365+366+365+365+365+366};
int days(int year, int month, int day)
{
	int extra=((year+1998)>>2)&1;
	
	if(year+1998>2005 || month>11 || day>30) {
		lg("Bad date %d %d %d\n",year+1998,month+1,day);
		return -1;
	}
	return yeardays[year]+monthdays[extra][month]+day;
}

// Minimize xAx -2bx with x>0
// Improved Neighborhood-based Collaborative Filtering
// Neighbor-Koren.pdf
// Figure1
#define MAXK (100)
int totalNonNegativeQuadraticOpt=0;
int problemNonNegativeQuadraticOpt=0;
NonNegativeQuadraticOpt(double *A, double *b, double *x, int k)
{
	totalNonNegativeQuadraticOpt++;
	if(k>=MAXK) error("K %d is too big\n",k);
	double w[MAXK],bestrr=INF,rr;
	int i;
	for(i=0;i<k;i++) w[i]=1/k; //initiall guess
	int itr;
	for(itr=0;itr<5000;itr++) {
		double r[MAXK]; // the residual, or "steepest gradient"
		double *a=A;
		for(i=0;i<k;i++) {
			int j;
			double sum=0.;
			for(j=0;j<k;j++)
				sum+=(*a++)*w[j];
			r[i]=b[i] - sum; //http://www.netflixprize.com/community/viewtopic.php?pid=6025#p6025
		}
		// find active variables - those that are pinned because of
		// nonnegativity constraint, and set respective ri’s to zero
		for(i=0;i<k;i++)
			if((w[i]<EPS)&&(r[i]<0.)) r[i]=0.;
		// max step size
		double rAr=0.;
		rr=0.;
		a=A;
		for(i=0;i<k;i++) {
			int j;
			double sum=0.;
			for(j=0;j<k;j++)
				sum+=(*a++)*r[j];
			rAr+=sum*r[i];
			rr+=r[i]*r[i];
		}
		double alpha=rr/rAr;
		//adjust step size to prevent negative values:
		//http://www.netflixprize.com/community/viewtopic.php?pid=6139#p6139
		if(isnan(alpha) || alpha<EPS) alpha=0.001;
		for(i=0;i<k;i++) {
			if (r[i] * alpha < -EPS ) { 
				alpha = (fabs(alpha) < fabs(w[i]/r[i])) ? fabs(alpha): fabs(w[i]/r[i]);
            } else if (r[i] * alpha > EPS)
				alpha = fabs(alpha);
		}
		for(i=0;i<k;i++) {
			w[i]+=alpha*r[i];
			if(w[i]<1.e-10) w[i]=0.; //http://www.netflixprize.com/community/viewtopic.php?pid=6025#p6025
		}

		if(rr<bestrr) {
			bestrr=rr;
			for(i=0;i<k;i++) x[i]=w[i];
		}
		if(rr<0.00005) return; //http://www.netflixprize.com/community/viewtopic.php?pid=6029#p6029
		/*if(rr>bestrr+1.) return; //http://www.netflixprize.com/community/viewtopic.php?pid=6030#p6030*/
	}
	/*if(rr<0.01) return;*/
	/*double *a=A;*/
	/*for(i=0;i<k;i++) {*/
		/*int j;*/
		/*for(j=0;j<k;j++)*/
			/*lg("%f,",*a++);*/
	/*}*/
	/*for(i=0;i<k;i++) {*/
		/*lg("%f,\n",b[i]);*/
	/*}*/
	/*error("Does not converage");*/
	problemNonNegativeQuadraticOpt++;
	lg("Does not converage %f %f %f\n",problemNonNegativeQuadraticOpt/(double)totalNonNegativeQuadraticOpt,bestrr,rr);
}

/* Generate a random permutation */
void randperm(int perm[], int d)
{
	int i;
	for(i=0;i<d;i++) perm[i]=i;
	i=d;
	while(i>1) {
		int j=lrand48()%i;
		i--;
		int t=perm[i];
		perm[i]=perm[j];
		perm[j]=t;
	}
}

// v is sorted
// find idx such that v[idx]<t<=v[idx+1] or -1
int dvsearch(double *v, int d, double t)
{
	// use binary search
	// http://en.wikipedia.org/wiki/Binary_search
	int low = 0;
	int high=d-1;
	while (low <= high) {
		int mid = (low + high)>>1;
		if (v[mid] < t)
			low = mid + 1;
		else if (t<=v[mid-1])
			high=mid-1;
		else
			return mid; 
	}
	lg("%d %d %f %f %f\n",low,d,v[low],t,v[low+1]);
	return -1;
}
int fvsearch(float *v, int d, double t)
{
	// use binary search
	// http://en.wikipedia.org/wiki/Binary_search
	int low = 0;
	int high=d-1;
	while (low <= high) {
		int mid = (low + high)>>1;
		if (v[mid] < t)
			low = mid + 1;
		else if (t<=v[mid-1])
			high=mid-1;
		else
			return mid; 
	}
	lg("%d %d %f %f %f\n",low,d,v[low],t,v[low+1]);
	return -1;
}

//  quickSort
//
//  This public-domain C implementation by Darel Rex Finley.
//
//  * This function assumes it is called with valid parameters.
//
//  * Example calls:
//    quickSort(&myArray[0],5); // sorts elements 0, 1, 2, 3, and 4
//    quickSort(&myArray[3],5); // sorts elements 3, 4, 5, 6, and 7


#define  MAX_LEVELS  (300)

void uquickSort(unsigned int *arr, int elements) {
	int  beg[MAX_LEVELS], end[MAX_LEVELS], i=0, L, R, swap ;
	unsigned piv;

	beg[0]=0; end[0]=elements;
	while (i>=0) {
		L=beg[i]; R=end[i]-1;
		if (L<R) {
			piv=arr[L];
			while (L<R) {
				while (arr[R]>=piv && L<R) R--;
				if (L<R) {
					arr[L++]=arr[R];
				}
				while (arr[L]<=piv && L<R) L++;
				if (L<R) {
					arr[R--]=arr[L];
				}
			}
			if(i>=MAX_LEVELS-1) error("sort overflow");
			arr[L]=piv; beg[i+1]=L+1; end[i+1]=end[i]; end[i++]=L;
			if (end[i]-beg[i]>end[i-1]-beg[i-1]) {
				swap=beg[i]; beg[i]=beg[i-1]; beg[i-1]=swap;
				swap=end[i]; end[i]=end[i-1]; end[i-1]=swap;
			}
		}
		else {
			i--;
		}
	}
}

void iquickSortIdx(int *arr, int *idx, int elements) {
	int  piv, beg[MAX_LEVELS], end[MAX_LEVELS], i=0, L, R, swap ;
	int ipiv;

	beg[0]=0; end[0]=elements;
	while (i>=0) {
		L=beg[i]; R=end[i]-1;
		if (L<R) {
			piv=arr[L];
			ipiv=idx[L];
			while (L<R) {
				while (arr[R]>=piv && L<R) R--;
				if (L<R) {
					idx[L]=idx[R];
					arr[L++]=arr[R];
				}
				while (arr[L]<=piv && L<R) L++;
				if (L<R) {
					idx[R]=idx[L];
					arr[R--]=arr[L];
				}
			}
			arr[L]=piv; idx[L]=ipiv; beg[i+1]=L+1; end[i+1]=end[i]; end[i++]=L;
			if (end[i]-beg[i]>end[i-1]-beg[i-1]) {
				swap=beg[i]; beg[i]=beg[i-1]; beg[i-1]=swap;
				swap=end[i]; end[i]=end[i-1]; end[i-1]=swap;
			}
		}
		else {
			i--;
		}
	}
}


void uquickSortIdx(unsigned int *arr, int *idx, int elements) {
	int  beg[MAX_LEVELS], end[MAX_LEVELS], i=0, L, R, swap ;
	unsigned int piv;
	int ipiv;

	beg[0]=0; end[0]=elements;
	while (i>=0) {
		L=beg[i]; R=end[i]-1;
		if (L<R) {
			piv=arr[L];
			ipiv=idx[L];
			while (L<R) {
				while (arr[R]>=piv && L<R) R--;
				if (L<R) {
					idx[L]=idx[R];
					arr[L++]=arr[R];
				}
				while (arr[L]<=piv && L<R) L++;
				if (L<R) {
					idx[R]=idx[L];
					arr[R--]=arr[L];
				}
			}
			arr[L]=piv; idx[L]=ipiv; beg[i+1]=L+1; end[i+1]=end[i]; end[i++]=L;
			if (end[i]-beg[i]>end[i-1]-beg[i-1]) {
				swap=beg[i]; beg[i]=beg[i-1]; beg[i-1]=swap;
				swap=end[i]; end[i]=end[i-1]; end[i-1]=swap;
			}
		}
		else {
			i--;
		}
	}
}

void fquickSortIdx(float *arr, int *idx, int elements) {
	int  beg[MAX_LEVELS], end[MAX_LEVELS], i=0, L, R, swap ;
	float piv;
	int ipiv;

	beg[0]=0; end[0]=elements;
	while (i>=0) {
		if(i>MAX_LEVELS-2) error("sort overflow");
		L=beg[i]; R=end[i]-1;
		if (L<R) {
			piv=arr[L];
			ipiv=idx[L];
			while (L<R) {
				while (arr[R]>=piv && L<R) R--;
				if (L<R) {
					idx[L]=idx[R];
					arr[L++]=arr[R];
				}
				while (arr[L]<=piv && L<R) L++;
				if (L<R) {
					idx[R]=idx[L];
					arr[R--]=arr[L];
				}
			}
			arr[L]=piv; idx[L]=ipiv; beg[i+1]=L+1; end[i+1]=end[i]; end[i++]=L;
			if (end[i]-beg[i]>end[i-1]-beg[i-1]) {
				swap=beg[i]; beg[i]=beg[i-1]; beg[i-1]=swap;
				swap=end[i]; end[i]=end[i-1]; end[i-1]=swap;
			}
		}
		else {
			i--;
		}
	}
}

void dquickSortIdx(double *arr, int *idx, int elements) {
	int  beg[MAX_LEVELS], end[MAX_LEVELS], i=0, L, R, swap ;
	double piv;
	int ipiv;

	beg[0]=0; end[0]=elements;
	while (i>=0) {
		L=beg[i]; R=end[i]-1;
		if (L<R) {
			piv=arr[L];
			ipiv=idx[L];
			while (L<R) {
				while (arr[R]>=piv && L<R) R--;
				if (L<R) {
					idx[L]=idx[R];
					arr[L++]=arr[R];
				}
				while (arr[L]<=piv && L<R) L++;
				if (L<R) {
					idx[R]=idx[L];
					arr[R--]=arr[L];
				}
			}
			arr[L]=piv; idx[L]=ipiv; beg[i+1]=L+1; end[i+1]=end[i]; end[i++]=L;
			if (end[i]-beg[i]>end[i-1]-beg[i-1]) {
				swap=beg[i]; beg[i]=beg[i-1]; beg[i-1]=swap;
				swap=end[i]; end[i]=end[i-1]; end[i-1]=swap;
			}
		}
		else {
			i--;
		}
	}
}

double gauss()
{
	double x1, x2, w, y1;
	static int flag=0;
	static double y2;
	if(flag) {
		flag=0;
		return y2;
	}
 
	do {
		x1 = 2.0 * drand48() - 1.0;
		x2 = 2.0 * drand48() - 1.0;
		w = x1 * x1 + x2 * x2;
	} while ( w >= 1.0 );

	w = sqrt( (-2.0 * log( w ) ) / w );
	y1 = x1 * w;
	y2 = x2 * w;
	flag=1;
	return y1;
}
