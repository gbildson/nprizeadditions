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

float wgt[NENTRIES];

void weight_time_setup()
{
	int i,u;
#if 0
	// Build day distribution for probe/qualify data
	int dwgt[MAX_DAY+1];
	ZERO(dwgt);
	for(u=0;u<NUSERS;u++) {
		int base=useridx[u][0];
		base+=useridx[u][1];
		int j;
		for(j=0;j<useridx[u][2]+useridx[u][3];j++)
			dwgt[userent[j+base]>>(USER_LDAY+4)]++;
	}
	// ALPHA
	for(i=0;i<=MAX_DAY;i++)
		dwgt[i]+=500;
		
	for(i=0;i<NENTRIES;i++)
		wgt[i]=dwgt[userent[i]>>(USER_LDAY+4)];
#else
	for(i=0;i<NENTRIES;i++)
		wgt[i]=(userent[i]>>USER_LDAY)+MAX_DAY;
#endif
}

void weight_norm()
{
	double sum=0.;
    int total=0;
    int u,i;
	for(u=0;u<NUSERS;u++) {
		int base=useridx[u][0];
		int d=UNTRAIN(u);
		int j;
		for(j=0;j<d;j++) {
			sum+=wgt[base+j];
		}
        total+=d;
	}
	lg("sum=%f\n",sum);
    
    sum=total/sum;
    for(i=0;i<NENTRIES;i++) wgt[i]*=sum;
}
