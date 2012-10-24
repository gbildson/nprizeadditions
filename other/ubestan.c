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
		double stepUmax=0,   stepUmin=99999,  stepUavg=0;
		double regUmax=0,    regUmin=99999,   regUavg=0;
		double errmax=0,     errmin=99999,    erravg=0;
		double totUmax=0,    totUmin=99999  , totUavg=0;
		double astepUmax=0,  astepUmin=99999, astepUavg=0;
		double aregUmax=0,   aregUmin=99999,  aregUavg=0;
		double aerrmax=0,    aerrmin=99999,   aerravg=0;
		double atotUmax=0,   atotUmin=99999  ,atotUavg=0;
		double pval=0, apval=0;
		int n=0;
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

				if ( e2 > errmax)
					errmax = e2;
				double ae2 = fabs(e2);
				if ( ae2 > aerrmax)
					aerrmax = ae2;
				if ( e2 < errmin )
					errmin= e2;
				if ( ae2 < aerrmin)
					aerrmin = ae2;
				erravg  += e2;
				aerravg += ae2;
				double stepU  = Gamma0 * e2;
				double astepU = fabs(stepU);
				double regU  = Gamma0 * wbVm * L4;
				//double regU  = Gamma0 * wbUu * L4;
				double aregU = fabs(regU);
				double totU  = stepU - regU;
				double atotU  = astepU - aregU;
				if (stepU > stepUmax)
					stepUmax = stepU;
				if (astepU > astepUmax)
					astepUmax = astepU;
				if (stepU < stepUmin)
					stepUmin = stepU;
				if (astepU < astepUmin)
					astepUmin = astepU;
				if (totU > totUmax)
					totUmax = totU;
				if (totU < totUmin)
					totUmin = totU;
				totUavg += totU;
				stepUavg += stepU;
				astepUavg += astepU;
				if (regU > regUmax)
					regUmax = regU;
				if (aregU > aregUmax)
					aregUmax = aregU;
				if (regU < regUmin)
					regUmin = regU;
				if (aregU < aregUmin)
					aregUmin = aregU;
				regUavg += regU;
				aregUavg += aregU;
				if (atotU > atotUmax)
					atotUmax = atotU;
				if (atotU < atotUmin)
					atotUmin = atotU;
				atotUavg += atotU;
				pval += wbVm;
				apval += fabs(wbVm);

				//printf("err: %11.6f\tstepU: %11.6f\tregU: %11.6f\ttotU: %11.6f\tr%%e:%11.6f\tr%%s: %11.6f\tr%%t: %11.6f\n", e2, stepU, regU, totU, 100*regU/e2, 100*regU/stepU, 100*regU/totU); 

				wbU[u] += Gamma0 * (e2 - wbUu * L4);
				wbV[m] += Gamma0 * (e2 - wbVm * L4);
				n++;
			}
		}
		stepUavg /= n;
		regUavg /= n;
		totUavg /= n;
		erravg /= n;
		astepUavg /= n;
		aregUavg /= n;
		aerravg /= n;
		atotUavg /= n;
		printf("Max err: %11.6f\tstepU: %11.6f\tregU: %11.6f\ttotU: %11.6f\tr%%e:%11.6f\tr%%s: %11.6f\tr%%t: %11.6f\ts%%e %11.6f\n", errmax, stepUmax, regUmax, totUmax, 100*regUmax/errmax, 100*regUmax/stepUmax, 100*regUmax/totUmax, 100*stepUmax/errmax); 
		printf("Min err: %11.6f\tstepU: %11.6f\tregU: %11.6f\ttotU: %11.6f\tr%%e:%11.6f\tr%%s: %11.6f\tr%%t: %11.6f\ts%%e %11.6f\n", errmin, stepUmin, regUmin, totUmin, 100*regUmin/errmin, 100*regUmin/stepUmin, 100*regUmin/totUmin, 100*stepUmin/errmin); 
		printf("AVG err: %11.6f\tstepU: %11.6f\tregU: %11.6f\ttotU: %11.6f\tr%%e:%11.6f\tr%%s: %11.6f\tr%%t: %11.6f\ts%%e %11.6f\n", erravg, stepUavg, regUavg, totUavg, 100*regUavg/erravg, 100*regUavg/stepUavg, 100*regUavg/totUavg, 100*stepUavg/erravg); 
		printf("aMax err: %11.6f\tstepU: %11.6f\tregU: %11.6f\ttotU: %11.6f\tr%%e:%11.6f\tr%%s: %11.6f\tr%%t: %11.6f\ts%%e %11.6f\n", aerrmax, astepUmax, aregUmax, atotUmax, 100*aregUmax/aerrmax, 100*aregUmax/astepUmax, 100*aregUmax/atotUmax, 100*astepUmax/aerrmax); 
		printf("aMin err: %11.6f\tstepU: %11.6f\tregU: %11.6f\ttotU: %11.6f\tr%%e:%11.6f\tr%%s: %11.6f\tr%%t: %11.6f\ts%%e %11.6f\n", aerrmin, astepUmin, aregUmin, atotUmin, 100*aregUmin/aerrmin, 100*aregUmin/astepUmin, 100*aregUmin/atotUmin, 100*astepUmin/aerrmin); 
		printf("aAVG err: %11.6f\tstepU: %11.6f\tregU: %11.6f\ttotU: %11.6f\tr%%e:%11.6f\tr%%s: %11.6f\tr%%t: %11.6f\ts%%e %11.6f\n", aerravg, astepUavg, aregUavg, atotUavg, 100*aregUavg/aerravg, 100*aregUavg/astepUavg, 100*aregUavg/atotUavg, 100*astepUavg/aerravg); 
		fflush(stdout);

		pval /= n;
		apval /= n;

	 	double newV_REG    = 0.0211 / 100.0 /** aerravg*/ / Gamma0 / apval;  
	 	double newV_LRATE  = 1.1100 / 100.0 /** aerravg*/;
	 	double newV_REG2   = newV_LRATE * 1.907 / 100.0 / apval /Gamma0;
	 	double newV_REG3   = aregUavg / astepUavg / apval;
	 	//double newV_REG4   = 0.01907 * aerravg / Gamma0 / apval;
	 	double newV_REG4   = 1.9074 / 100.0 * astepUavg / Gamma0 / apval;
				//double stepU  = Gamma0 * e2;
				//double regU  = Gamma0 * wbVm * L4;
//#define G0 (0.0111) // for biases
//#define L4 (0.0450) // for biases
		printf("\n  pval: %f\tapval: %f", pval, apval);
		printf("\tnew REG: %f\tNew LRATE: %f\tREG2: %f\tREG4: %f\n\n", newV_REG, newV_LRATE, newV_REG2, newV_REG4);

// Max err:    4.933245    stepU:    0.054759      regU:    0.000873       totU:    0.055966       r%e:   0.017702 r%s:    1.594802        r%t:    1.560400        s%e    1.110000
// Min err:   -4.797758    stepU:   -0.053255      regU:   -0.001418       totU:   -0.053857       r%e:   0.029560 r%s:    2.663103        r%t:    2.633345        s%e    1.110000
// AVG err:    0.002357    stepU:    0.000026      regU:   -0.000055       totU:    0.000081       r%e:  -2.312720 r%s: -208.353166        r%t:  -67.569654        s%e    1.110000
// aMax err:    4.933245   stepU:    0.054759      regU:    0.001418       totU:    0.053552       r%e:   0.028749 r%s:    2.589963        r%t:    2.648351        s%e    1.110000
// aMin err:    0.000000   stepU:    0.000000      regU:    0.000000       totU:   -0.001389       r%e:   0.000000 r%s:    0.000000        r%t:   -0.000000        s%e    0.011100
// aAVG err:    0.711899   stepU:    0.007902      regU:    0.000148       totU:    0.007754       r%e:   0.020762 r%s:    1.870466        r%t:    1.906119        s%e    0.011100

		// Report rmse for main loop
		nrmse=0.;
		int ntrain=0;
		int elcnt=0;
		int k=2;
		n=0;
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
		
		lg("%11.6f\t%11.6f\t%11.6f\n",nrmse,prmse,(clock()-t0)/(double)CLOCKS_PER_SEC);

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

