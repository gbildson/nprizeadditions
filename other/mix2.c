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
#define LAMBDA (20.)
#define HOLDOUT
/*
mix scr.bin files generated with the "-se" flag in utestX
we want to find mixture coefficents Bj
Ri = sumj Rij Bj

that minimize the L2 norm of the error

sumi Ri^2

on the training data under the constrain that sumj Bj=1

sumi Ri^2 + L sumj Bj

difreniating gives

sumi 2 Rij (sumj Rij Bj) + L =0
or replacing L with l=-L/2:
A B = l where  Ajk= sumi Rji Rki
since this is a linear equation we can replace l with 1 and normalize B latter.


these equations are solved using LAPACK library.
You need to use Cygwin's setup tool and download the lapack package from the
Math category.

 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "basic.h"
#include "netflix.h"
#include "utest.h"

#define NSCORES (5)

void openfiles(FILE *fp[],char *fnames[], int nscores)
{
	int f;
	for(f=0;f<nscores;f++) {
		lg("Mixing %s\n",fnames[f]);
		if(NULL==(fp[f]=fopen(fnames[f],"rb")))
			error("Cant open");
	}
}

void seekfiles(FILE *fp[], int nscores, int d)
{
	if(!d) return;
	int f;
	for(f=0;f<nscores;f++)
		if(fseek(fp[f],d*sizeof(float),SEEK_CUR))
			error("Failed to seek\n");
}

void readfiles(FILE *fp[], float *s, int nscores)
{
	int f;
	for(f=0;f<nscores;f++)
		if(1!=fread(&s[f],sizeof(float),1,fp[f])) {
			lg("Error\n");
			exit(1);
		}
}

void closefiles(FILE *fp[], int nscores)
{
	int f;
	for(f=0;f<nscores;f++)
		fclose(fp[f]);
}

computemix(char *fnames[], int nscores, double *xty)
{
#ifdef HOLDOUT
	lg("With holdout\n");
#endif
	int ns2=nscores+2;
	int ns1=nscores+1;
	FILE *fp[NSCORES];
	openfiles(fp,fnames,nscores);

	double xtx[NSCORES+2][NSCORES+2];
	ZERO(xtx);
	
	int u;
	for(u=0; u<NUSERS; u++) {
		PROGRESS(u,NUSERS);
		int base=useridx[u][0];
#ifdef HOLDOUT
		if(aopt) error("cant do holdout with -a");
		int d0=UNTRAIN(u);
		int d1=UNALL(u)-d0;
#else
		int d0=0;
		int d1=UNTRAIN(u);
#endif
		seekfiles(fp,nscores, d0);
		base+=d0;
		int j;			
		for(j=0;j<d1;j++) {
			unsigned int dd=userent[base+j];
			int r = (dd>>USER_LMOVIEMASK)&7;
			float s[NSCORES+2];
			readfiles(fp,s,nscores);
			int f;
			for(f=0;f<nscores;f++)		
				s[f]=r-s[f];
			s[nscores]=1.;
			s[nscores+1]=r;

			int ff;
			for(f=0;f<ns2;f++) {
				for(ff=0;ff<ns2;ff++)
					xtx[f][ff] +=s[f]*s[ff];
			}
		}
		int d2=UNTOTAL(u)-(d1+d0);
		seekfiles(fp,nscores, d2);
	}
	closefiles(fp,nscores);
	int count=xtx[nscores][nscores];
	int j1,j2;
	for(j1=0;j1<nscores;j1++)
		lg("File %d RMSE %f\n",j1,sqrt((xtx[j1][j1]+xtx[ns1][ns1]-2*xtx[ns1][j1])/count));
	double avgs[NSCORES+2],std[NSCORES+2];
	for(j1=0;j1<ns2;j1++) {
		avgs[j1]=xtx[nscores][j1]/count;
		std[j1]=sqrt(xtx[j1][j1]/count-avgs[j1]*avgs[j1]);
	}
	for(j1=0;j1<ns2;j1++)
		lg("%f\t",avgs[j1]);
	lg("\n");
	for(j1=0;j1<ns2;j1++)
		lg("%f\t",std[j1]);
	lg("\n");
	lg("-------------------------------------------------\n");
	for(j1=0;j1<ns2;j1++) {
		for(j2=0;j2<ns2;j2++) {
			lg("%f\t",(xtx[j1][j2]/count-avgs[j1]*avgs[j2])/(std[j1]*std[j2]+1.e-6));
		}
		lg("\n");
	}
	lg("-------------------------------------------------\n");
	double eavgs[NSCORES],estd[NSCORES];
	for(j1=0;j1<nscores;j1++) {
		eavgs[j1]=avgs[ns1]-avgs[j1];
		estd[j1]=sqrt((xtx[ns1][ns1]+ xtx[j1][j1]-2*xtx[j1][ns1])/count);
	}
	for(j1=0;j1<nscores;j1++)
		lg("%f\t",eavgs[j1]);
	lg("\n");
	for(j1=0;j1<nscores;j1++)
		lg("%f\t",estd[j1]);
	lg("\n");
	lg("-------------------------------------------------\n");
	for(j1=0;j1<nscores;j1++) {
		for(j2=0;j2<nscores;j2++) {
			lg("%f\t",((xtx[j1][j2]+xtx[ns1][ns1]-xtx[ns1][j1]-xtx[ns1][j2])/count-eavgs[j1]*eavgs[j2])/(estd[j1]*estd[j2]+1.e-6));
		}
		lg("\n");
	}


	char TRANS='N';
	char UFLO='U';
	int M=ns1;
	int N=ns1;
	int NRHS=1;
	double A[NSCORES+1][NSCORES+1];
	int LDA=NSCORES+1;
	double B[NSCORES+1];
	int LDB=NSCORES+1;
	double S[NSCORES+1];
	double RCOND=0.00001; // singular values below this are treated as zero.
	int RANK;
	double WORK[1000];
	int LWORK=1000;
	int INFO;

	for(j1=0;j1<ns1;j1++) {
		B[j1]=xtx[ns1][j1];
		for(j2=0;j2<ns1;j2++)
			A[j1][j2]=xtx[j1][j2];
	}	
	for(j1=0;j1<ns1;j1++) A[j1][j1]+=LAMBDA;
	/*dgesv_(&N,&NRHS,A,&LDA,IPIV,B,&LDB,&INFO);*/
	/*dgels_(&TRANS,&M,&N,&NRHS,A,&LDA,B,&LDB,WORK,&LWORK,&INFO);*/
	/*dgelss_( &M, &N, &NRHS, A, &LDA, B, &LDB, S, &RCOND, &RANK, WORK, &LWORK, &INFO );*/
	dposv_(&UFLO,&N,&NRHS,A,&LDA,B,&LDB,&INFO);
	if(INFO) error("failed %d\n",INFO);
		
	for(j1=0;j1<=nscores;j1++)
		xty[j1]=B[j1];

	lg("Check that the matrix inversion worked:\n");
	for(j1=0;j1<=nscores;j1++) {
		double sum=LAMBDA*B[j1];
		for(j2=0;j2<=nscores;j2++)
			sum+=xtx[j1][j2]*B[j2];
		lg("%f\t%f\n",sum,xtx[nscores+1][j1]);
	}
}

loadmix(char *fnames[], int nscores, double *weights) {
	if(nscores<2 || nscores>NSCORES) error("Bad number of files\n");
	
	double xty[NSCORES+1];
	if(weights) {
		int j;
		for(j=0;j<=nscores;j++)
			xty[j]=weights[j];
	} else
		computemix(fnames, nscores, xty);
	lg("Mixing coeeficients\n");
	int f;
	for(f=0;f<=nscores;f++)
		lg("-lew %f ",xty[f]);
	lg("\n");
		
	FILE *fp[NSCORES];
	openfiles(fp,fnames,nscores);
	int i;
	for(i=0; i<NENTRIES; i++) {
		PROGRESS(i,NENTRIES);
		int r=(userent[i]>>USER_LMOVIEMASK)&7;
		float s[NSCORES];
		readfiles(fp,s,nscores);
		float stotal=0.;
		int j;
		for(j=0;j<nscores;j++)
			stotal+=xty[j]*(r-s[j]);
		stotal+=xty[nscores];
		err[i]=r-stotal;
	}
	closefiles(fp,nscores);
}
