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
#define ZERO(v) memset(v,0,sizeof(v))
void load_bin(char *path, void *data, int len);
int dload_bin(char *fname,double *vec,int M,int N1);

void dump_bin(char *path, void *data, int len);
void ddump_bin(char *fname,double *vec,int M,int N,int N1);

int days(int year, int month, int day);

unsigned int uivmin(unsigned int *v, int n);
double dvavg(double *v, int n);
double fdvdot(float *v1, double *v2, int n);
double ddvdot(double *v1, double *v2, int n);
double fdvwdot(float *v1, double *v2, int n,double *wgt);
double dvsqr(double *v, int n);
double dvwsqr(double *v, int n, double *wgt);
double fvsqr(float *v, int n);
double gauss();
    
#define EPS (1.e-20)
#define INF (1.e20)
#define PROGRESS(i,N)	if(!(i%(1+(N/100)))) lg("%d%%\r",(int)((100.*i)/(double)N))
#define PROGRESS1(i,N)	if(!(i%(1+(N/1000)))) lg("%.1f%%\r",((100.*i)/N))

int dvsearch(double *v, int d, double t);
int fvsearch(float *v, int d, double t);
void randperm(int perm[], int d);

void dquickSortIdx(double *arr, int *idx, int elements);
void fquickSortIdx(float *arr, int *idx, int elements);
void uquickSortIdx(unsigned int *arr, int *idx, int elements);
void iquickSortIdx(int *arr, int *idx, int elements);
void uquickSort(unsigned int *arr, int elements);
