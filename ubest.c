/*
########################################################################
#  Netflix Prize Tools
#  Copyright (C) 2009 Greg Bildson
#  http://code.google.com/p/nprizeadditions/
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
/*   ubest.c
     This algorithm determines the baseline estimates bU and bV as used in the integrated model.

     For documentation, see section 2.1 here: 
	   http://public.research.att.com/~volinsky/netflix/kdd08koren.pdf
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
int score_argv(char **argv) {return 0;}

#define GLOBAL_MEAN (3.603304)
float wbU[NUSERS]; 
float wbV[NMOVIES];

#define G0 (0.0111) // for biases
#define L4 (0.0450) // for biases

void score_setup() {
}

void removeUV() {
	int u,f;
	for(u=0;u<NUSERS;u++) {
		int base0=useridx[u][0];
		int d012=UNALL(u);
		int i;
		int dall=UNALL(u);

		int j,j2;

		for(i=0; i<d012;i++) {
			int m=userent[base0+i]&USER_MOVIEMASK;

			int r=(userent[base0+i]>>USER_LMOVIEMASK)&7;
			r++;

			err[base0+i] = r - (GLOBAL_MEAN + wbU[u] + wbV[m]);
		}
	}
}

int score_train(int loop) {
	if (loop == 0)
		return doAllFeatures();
	
	return 1;
}

int doAllFeatures() {

	/* Initial biases */
	{
		int u,m;
		
		for(u=0;u<NUSERS;u++) {
			wbU[u]=0.0;
		}
		for(m=0;m<NMOVIES;m++) {
			wbV[m]=0.0;
		}
	}
	
    float swbU[NUSERS]; 
    float swbV[NMOVIES];
	
	/* Optimize current feature */
	float nrmse=2., last_rmse=10.;
	float prmse = 0, last_prmse=0;
	int loopcount=0;
	float Gamma0 = G0;
	while( ( (prmse<=last_prmse) || loopcount < 6) ) {
		last_rmse=nrmse;
		last_prmse=prmse;
		nrmse=0.;
		clock_t t0=clock();
	    loopcount++;

		int u,m,j;

		// Save the prior wbU and wbV for when the RMSE gets worse
		for(u=0;u<NUSERS;u++) {
			swbU[u] = wbU[u];
		}
		for(m=0;m<NMOVIES;m++) {
			swbV[m]=wbV[m];
		}

		// Train
		for(u=0;u<NUSERS;u++) {

			//if (u%10000 == 0) {
				//printf("On user: %d\n", u);
				//fflush(stdout);
			//}

			int d0 = UNTRAIN(u);
			int base0=useridx[u][0];

			// For all rated movies
			for(j=0;j<d0;j++) {
				int m=userent[base0+j]&USER_MOVIEMASK;

				// Figure out the current error
			    int r=(userent[base0+j]>>USER_LMOVIEMASK)&7;
			    r++;
				//float ee=err[base0+j];
				//float e2 = ee;
				float e2;
				e2 = r - (GLOBAL_MEAN + wbU[u] + wbV[m]);

				// Train the biases
				float wbUu = wbU[u];
				float wbVm = wbV[m];
				wbU[u] += Gamma0 * (e2 - wbUu * L4);
				wbV[m] += Gamma0 * (e2 - wbVm * L4);
			}
		}

		// Report rmse for main loop
		nrmse=0.;
		int ntrain=0;
		int elcnt=0;
		int k=2;
		int n=0;
		float s=0.;
		int i;
		for(u=0;u<NUSERS;u++) {

			int base0=useridx[u][0];
			int d012=UNALL(u);
			int i;
			int dall=UNALL(u);
			int d0 = UNTRAIN(u);

			for(i=0; i<d0;i++) {
				int m=userent[base0+i]&USER_MOVIEMASK;

			    int r=(userent[base0+i]>>USER_LMOVIEMASK)&7;
			    r++;
				//float ee=err[base0+j];
				float e2;
				e2 = r - (GLOBAL_MEAN + wbU[u] + wbV[m]);

				nrmse+=e2*e2;
				ntrain++;
			}


			// Attempt to compute probe RMSE
			int base=useridx[u][0];
			for(i=1;i<k;i++) base+=useridx[u][i];
			int d=useridx[u][k];
			int boffset = base-base0;
			for(i=0; i<d;i++) {
				int m=userent[base+i]&USER_MOVIEMASK;

				//float ee = err[base+i];
				float e;
			    int r=(userent[base+i]>>USER_LMOVIEMASK)&7;
			    r++;
				e = r - (GLOBAL_MEAN + wbU[u] + wbV[m]);

				s+=e*e;
			}
			n+=d;
		}

		nrmse=sqrt(nrmse/ntrain);
		prmse = sqrt(s/n);
		
		lg("%f\t%f\t%f\n",nrmse,prmse,(clock()-t0)/(double)CLOCKS_PER_SEC);

		Gamma0 *= 0.90;
	}

	// Restore the best parameters
	int u, m;
	for(u=0;u<NUSERS;u++) {
		wbU[u] = swbU[u];
	}
	for(m=0;m<NMOVIES;m++) {
		wbV[m] = swbV[m];
	}
	
	/* Perform a final iteration in which the errors are clipped and stored */
	removeUV();
	
	return 1;
}
