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


#define TOTAL_FEATURES  100
#define TOTAL_INPUTS    5*NMOVIES
#define epsilonw        0.01   // Learning rate for weights
#define epsilonvb       0.01   // Learning rate for biases of visible units
#define epsilonhb       0.01   // Learning rate for biases of hidden units
#define weightcost      0.02
#define momentum        0.0  //NO MOMENTUM
#define MAX_EPOCH       50


//double vishid[TOTAL_FEATURES*TOTAL_INPUTS];
double vishid[NMOVIES][5][TOTAL_FEATURES];
double CDpos[TOTAL_FEATURES*TOTAL_INPUTS];
double CDneg[TOTAL_FEATURES*TOTAL_INPUTS];
double visbiases[TOTAL_INPUTS];
double hidbiases[TOTAL_FEATURES];
double poshidprobs[TOTAL_FEATURES];
char  poshidstates[TOTAL_FEATURES]; 
double poshidact[TOTAL_FEATURES];
double neghidact[TOTAL_FEATURES];
double neghidprobs[TOTAL_FEATURES];
double negvisprobs[TOTAL_INPUTS];
//char  negvisstates[TOTAL_INPUTS]; 
char  negvissoftmax[NMOVIES]; 
unsigned int moviercount[5*NMOVIES];
unsigned int moviecount[NMOVIES];
double posvisact[TOTAL_INPUTS];
double negvisact[TOTAL_INPUTS];


//Customers have their own set of hidden units:
//
//struct Customer{
     //HiddenUnit posHiddenUnits[total_features];
     //HiddenUnit negHiddenUnits[total_features];
//}
//
//struct HiddenUnit{
     //char state;
     //double probability;
//} Sj[TOTAL_FEATURES];
//
//struct VisibleUnit{
     //char state[5];
     //double probabilities[5];
//}




#define E (0.00020) // stop condition
#define E2 (0.00002) // stop condition
int score_argv(char **argv) {return 0;}


#if 0
#define NFEATURES (50)
#define GLOBAL_MEAN (3.6033)

//double sU[NUSERS][NFEATURES];
//double sV[NMOVIES][NFEATURES];
//double sY[NMOVIES][NFEATURES];
//double bU[NUSERS]; 
//double bV[NMOVIES];

//#define G1 (0.007) 
//#define G2 (0.007) 
//#define L6 (0.005) // for biases
//#define L7 (0.015) 
#define G1 (0.0111) 
#define G2 (0.0111) 
#define L6 (0.045) // for biases
#define L7 (0.045) 
#define LbU (0.090050)
#define LbV (0.141838)
//#define LsU (0.052207)
//#define LsV (0.002846)
#define LsU (0.0275265)
#define LsV (0.0275265)
#define LsY (0.037649)
//bU: 0.090050 bV: 0.141838, sU: 0.052207, sV: 0.002846, sY: 0.037649
#endif


void score_setup()
{
	int i,u,m, j;

    for (m=0; m<NMOVIES; m++) {
		moviercount[m*5+0] = 0;
		moviercount[m*5+1] = 0;
		moviercount[m*5+2] = 0;
		moviercount[m*5+3] = 0;
		moviercount[m*5+4] = 0;
		moviecount[m] = 0;
	}
	for(u=0;u<NUSERS;u++) {
		int base0=useridx[u][0];
		int d0=UNTRAIN(u);
		//int dall=UNALL(u);

		// For all rated movies
		for(j=0;j<d0;j++) {
			int m=userent[base0+j]&USER_MOVIEMASK;
			int r=(userent[base0+j]>>USER_LMOVIEMASK)&7;
			moviercount[m*5+r]++;
			moviecount[m]++;
		}
	}
}


#if 0
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
		int j;
		for(j=0;j<dall;j++) {
			int mm=userent[base0+j]&USER_MOVIEMASK;
			for(f=0;f<NFEATURES;f++)
				sumY[f]+=sY[mm][f];
		}
		int d0=UNTRAIN(u);
		for(f=0;f<NFEATURES;f++) 
			lNuSY[f] = NuS * sumY[f]; 


        double bUu=bU[u];
		for(i=0; i<d012;i++) {
			int m=userent[base0+i]&USER_MOVIEMASK;
			err[base0+i]-=(bU[u] + bV[m]);
			for (f=0; f<NFEATURES; f++)
			    err[base0+i]-=((sU[u][f] + lNuSY[f]) * sV[m][f]);
		}
	}
}
#endif

int score_train(int loop)
{
	if (loop == 0)
		return doAllFeatures();
	
	return 1;
}

double randn() {
	return (rand()/(double)(RAND_MAX));
}

int doAllFeatures()
{
	/* Initial weights */
	int i, j, h;
    for (j=0; j<NMOVIES; j++) {
	    for (i=0; i<TOTAL_FEATURES; i++) {
		    //vishid[j*TOTAL_FEATURES+i]= 0.1 * rand(); // Normal Distribution
			//vishid[(j*5+0)*TOTAL_FEATURES+i] = 0.1 * rand(); // Normal Distribution
			vishid[j][0][i] = 0.1 * randn(); // Normal Distribution
			vishid[j][1][i] = 0.1 * randn(); // Normal Distribution
			vishid[j][2][i] = 0.1 * randn(); // Normal Distribution
			vishid[j][3][i] = 0.1 * randn(); // Normal Distribution
			vishid[j][4][i] = 0.1 * randn(); // Normal Distribution
	    }
	}

	/* Initial biases */
	for(i=0;i<TOTAL_FEATURES;i++) {
		hidbiases[i]=0.0;
	}
    //for (j=0; j<TOTAL_INPUTS; j++) 
    for (j=0; j<NMOVIES; j++) {
		//visbiases[j]= 0.0;
		//movie->visBias[0] = log( movie->numOnesForMovie / movie->RatingCount );

		unsigned int mtot = moviercount[j*5+0] + moviercount[j*5+1] + moviercount[j*5+2] + moviercount[j*5+3] + moviercount[j*5+4];
	    for (i=0; i<5; i++) {
		    //visbiases[j*5+i] = 0.0;
		    //visbiases[j*5+i] = log( ((double)moviercount[j*5+i]) / ((double) moviecount[j]) );
		    visbiases[j*5+i] = log( ((double)moviercount[j*5+i]) / ((double) mtot) );
//printf("mrc: %d, mc %d, log:%f frac: %f\n", moviercount[j*5+i], moviecount[j] , log( moviercount[j*5+i] /(double) moviecount[j]), 
//(moviercount[j*5+i] /(double) moviecount[j]) );
		}
	}


	
	/* Optimize current feature */
	double nrmse=2., last_rmse=10.;
	double prmse = 0, last_prmse=0;
	double thr=sqrt(1.-E);
	int loopcount=0;
	//double Gamma1 = G1;
	//double Gamma2 = G2;
	double EpsilonW  = epsilonw;
	double EpsilonVB = epsilonvb;
	double EpsilonHB =  epsilonhb;
	while( ((nrmse < (last_rmse-E)) /*&& prmse<last_prmse) */ || loopcount < 15) && loopcount < 200  )  {
		last_rmse=nrmse;
		last_prmse=prmse;
		clock_t t0=clock();
		loopcount++;
		int ntrain = 0;


		//* CDpos =0, CDneg=0 (matrices)
		ZERO(CDpos);
		ZERO(CDneg);
		ZERO(poshidact);
		ZERO(neghidact);
		ZERO(posvisact);
		ZERO(negvisact);
		int u,m, f;
		for(u=0;u<NUSERS;u++) {

			ZERO(negvisprobs);

		    //* perform steps 1 to 8

   			// 1. get one data point from data set.
   			// 2. use values of this data point to set state of visible neurons Si

   			// 3. compute Sj for each hidden neuron based on formula above and states of visible neurons Si
			// poshidprobs[h] = 1./(1 + exp(-V*vishid - hidbiases);
			int base0=useridx[u][0];
			int d0=UNTRAIN(u);

			// For all rated movies accumulate contributions to hidden units
			double sumW[TOTAL_FEATURES];
			ZERO(sumW);
			for(j=0;j<d0;j++) {
				int m=userent[base0+j]&USER_MOVIEMASK;
				int r=(userent[base0+j]>>USER_LMOVIEMASK)&7;
 
				// for all hidden units h:
				for(h=0;h<TOTAL_FEATURES;h++) {
        			// sum_j(W[i][j] * v[0][j]))
			    	//sumW[h]  += vishid[m*5+r)*TOTAL_FEATURES+h];
			    	sumW[h]  += vishid[m][r][h];
				}
			}
			// for all hidden units h:
			for(h=0;h<TOTAL_FEATURES;h++) {
				// compute Q(h[0][i] = 1 | v[0]) # for binomial units, sigmoid(b[i] + sum_j(W[i][j] * v[0][j]))
				poshidprobs[h]  = 1.0/(1.0 + exp(-sumW[h] - hidbiases[h]));
				// sample h[0][i] from Q(h[0][i] = 1 | v[0])
				if  ( poshidprobs[h] >  (rand()/(double)(RAND_MAX)) ) {
					poshidstates[h]=1;
				} else {
					poshidstates[h]=0;
				}
				poshidact[h] += poshidprobs[h];
			}

			// 5. on visible neurons compute Si using the Sj computed in step3. This is known as reconstruction
			// for all visible units j:
			int r;
			for(j=0;j<d0;j++) {
				int m=userent[base0+j]&USER_MOVIEMASK;
				for(h=0;h<TOTAL_FEATURES;h++) {
				    for(r=0;r<5;r++) {
//printf("phs: %d, vishid: %f\n", poshidstates[h] , vishid[m][r][h]);
			    	    negvisprobs[m*5+r]  += poshidstates[h] * vishid[m][r][h];
					}
				}
//printf("SUMS nvp: %f, nvp: %f, nvp: %f, nvp: %f, nvp: %f, m: %d\n", 
//negvisprobs[m*5+0],
//negvisprobs[m*5+1],
//negvisprobs[m*5+2],
//negvisprobs[m*5+3],
//negvisprobs[m*5+4],m);

//printf("exp parts - nn: %f, vb: %f, exp: %f\n", -negvisprobs[m*5+0],  -visbiases[m*5+0], exp(-negvisprobs[m*5+0] - visbiases[m*5+0]));

        	    // compute P(v[1][j] = 1 | h[0]) # for binomial units, sigmoid(c[j] + sum_i(W[i][j] * h[0][i]))
				negvisprobs[m*5+0]  = 1./(1 + exp(-negvisprobs[m*5+0] - visbiases[m*5+0]));
				negvisprobs[m*5+1]  = 1./(1 + exp(-negvisprobs[m*5+1] - visbiases[m*5+1]));
				negvisprobs[m*5+2]  = 1./(1 + exp(-negvisprobs[m*5+2] - visbiases[m*5+2]));
				negvisprobs[m*5+3]  = 1./(1 + exp(-negvisprobs[m*5+3] - visbiases[m*5+3]));
				negvisprobs[m*5+4]  = 1./(1 + exp(-negvisprobs[m*5+4] - visbiases[m*5+4]));
//printf("RAW nvp: %f, nvp: %f, nvp: %f, nvp: %f, nvp: %f, m: %d\n", 
//negvisprobs[m*5+0],
//negvisprobs[m*5+1],
//negvisprobs[m*5+2],
//negvisprobs[m*5+3],
//negvisprobs[m*5+4],m);

				// Normalize probabilities
				double tsum  = 
				  negvisprobs[m*5+0] +
				  negvisprobs[m*5+1] +
				  negvisprobs[m*5+2] +
				  negvisprobs[m*5+3] +
				  negvisprobs[m*5+4];
//printf("BEFORE nvp: %f, nvp: %f, nvp: %f, nvp: %f, nvp: %f, m: %d, tsum: %f\n", 
//negvisprobs[m*5+0],
//negvisprobs[m*5+1],
//negvisprobs[m*5+2],
//negvisprobs[m*5+3],
//negvisprobs[m*5+4],m, tsum);
				if ( tsum != 0 ) {
					negvisprobs[m*5+0]  /= tsum;
					negvisprobs[m*5+1]  /= tsum;
					negvisprobs[m*5+2]  /= tsum;
					negvisprobs[m*5+3]  /= tsum;
					negvisprobs[m*5+4]  /= tsum;
				}
//printf("nvp: %f, nvp: %f, nvp: %f, nvp: %f, nvp: %f, m: %d, tsum: %f\n", 
//negvisprobs[m*5+0],
//negvisprobs[m*5+1],
//negvisprobs[m*5+2],
//negvisprobs[m*5+3],
//negvisprobs[m*5+4],m, tsum);

        	    // sample v[1][j] from P(v[1][j] = 1 | h[0])
				double randval = (rand()/(double)(RAND_MAX));
				//negvisstates[m*5+0]) = 0;
				//negvisstates[m*5+1]) = 0;
				//negvisstates[m*5+2]) = 0;
				//negvisstates[m*5+3]) = 0;
				//negvisstates[m*5+4]) = 0;
				if ( (randval -= negvisprobs[m*5+0]) <= 0.0 )
				    negvissoftmax[m] = 0;
				    //negvisstates[m*5+0] = 1;
				else if ( (randval -= negvisprobs[m*5+1]) <= 0.0 )
				    negvissoftmax[m] = 1;
				    //negvisstates[m*5+1] = 1;
				else if ( (randval -= negvisprobs[m*5+2]) <= 0.0 )
				    negvissoftmax[m] = 2;
				    //negvisstates[m*5+2] = 1;
				else if ( (randval -= negvisprobs[m*5+3]) <= 0.0 )
				    negvissoftmax[m] = 3;
				    //negvisstates[m*5+3] = 1;
				else //if ( (randval -= negvisprobs[m*5+4]) <= 0.0 )
				    negvissoftmax[m] = 4;
				    //negvisstates[m*5+4] = 1;
				negvisact[m*5+0] += negvisprobs[m*5+0];
				negvisact[m*5+1] += negvisprobs[m*5+1];
				negvisact[m*5+2] += negvisprobs[m*5+2];
				negvisact[m*5+3] += negvisprobs[m*5+3];
				negvisact[m*5+4] += negvisprobs[m*5+4];
			}


			// 6. compute state of hidden neurons Sj again using Si from 5 step.
			// For all rated movies accumulate contributions to hidden units from sampled visible units
			ZERO(sumW);
			for(j=0;j<d0;j++) {
				int m=userent[base0+j]&USER_MOVIEMASK;
 
				// for all hidden units h:
				for(h=0;h<TOTAL_FEATURES;h++) {
			    	//sumW[h]  += vishid[(m*5+negvissoftmax[m])*TOTAL_FEATURES+h];
			    	sumW[h]  += vishid[m][negvissoftmax[m]][h];
				}
			}
			// for all hidden units h:
			for(h=0;h<TOTAL_FEATURES;h++) {
			    // compute Q(h[1][i] = 1 | v[1]) # for binomial units, sigmoid(b[i] + sum_j(W[i][j] * v[1][j]))
				neghidprobs[h]  = 1./(1 + exp(-sumW[h] - hidbiases[h]));
				neghidact[h] += neghidprobs[h];
			}


			// Update weights and biases and sum up the error
			for(j=0;j<d0;j++) {
				int m=userent[base0+j]&USER_MOVIEMASK;
				int r=(userent[base0+j]>>USER_LMOVIEMASK)&7;
				posvisact[m*5+r] += 1.0;
 
				// for all hidden units h:
				for(h=0;h<TOTAL_FEATURES;h++) {
					// for all softmax
					int rr;
				    for(rr=0;rr<5;rr++) {
						//double CDpos = 0.0;
						//double CDneg = 0.0;
						if ( poshidstates[h] == 1 ) {
    						//* accumulate CDpos = CDpos + (Si.Sj)0
							if ( r == rr )
							    CDpos[(m*5+r)*TOTAL_FEATURES+h] += 1.0;
								//CDpos += 1.0;
							//poshidact[h] += 1.0;
						}
					    //negvisact[m*5+rr] += negvisprobs[m*5+rr];
					    //negvisact[m*5+negvissoftmax[m]] += 1.0;

						//CDneg[(m*5+negvissoftmax[m])*TOTAL_FEATURES+h] += neghidprobs[h];
    					//* accumulate CDneg = CDneg + (Si.Sj)n
						//if ( rr == negvissoftmax[m] )
						    //CDneg += 1.0;
						//CDneg += neghidprobs[h]*negvisprobs[m*5+rr];
						CDneg[(m*5+rr)*TOTAL_FEATURES+h] += neghidprobs[h]*negvisprobs[m*5+rr];

						// W += epsilon * (h[0] * v[0]' - Q(h[1][.] = 1 | v[1]) * v[1]')
//float preW = vishid[m][rr][h];
						//vishid[m][rr][h] += EpsilonW * ((CDpos - CDneg) + weightcost * vishid[m][rr][h]);
//printf("W: %f preW: %f, CDp: %f, CDn: %f, m: %d, r: %d, h: %d, nhp: %f, nvp: %f\n", vishid[m][rr][h], preW, CDpos, CDneg, m, rr, h,
//neghidprobs[h],negvisprobs[m*5+rr]
//);
					}
					//if ( poshidstates[h] == 1 ) {
						//poshidact[h] += 1.0;
					//}
					//poshidact[h] += poshidprobs[h];
					//neghidact[h] += neghidprobs[h];
				}
			}

			// 4. now Si and Sj values can be used to compute (Si.Sj)0  here () means just values not average
			// 7. now use Si and Sj to compute (Si.Sj)1 (fig.3)
			// 8. repeating multiple times steps 5,6 and 7 compute (Si.Sj)n. Where n is small number and can 
			//    increase with learning steps to achieve better accuracy.

    	    // W += epsilon * (h[0] * v[0]' - Q(h[1][.] = 1 | v[1]) * v[1]')
			//vishid[(m*5+r)*TOTAL_FEATURES+h] += epsilonw * (CDpos[(m*5+r)*TOTAL_FEATURES+h] - CDneg[(m*5+r)*TOTAL_FEATURES+h]);
			
#if 0
    	    // b += epsilon * (h[0] - Q(h[1][.] = 1 | v[1]))
			for(h=0;h<TOTAL_FEATURES;h++) {
    	        hidbiases[h] += EpsilonHB / (d0*5) * ((poshidact[h] - neghidact[h]) + weightcost * hidbiases[h]);
//printf("hb: %f, pa: %f, na: %f, d0:%d\n", hidbiases[h], poshidact[h], neghidact[h], d0);
			}
#endif

    	    // c += epsilon * (v[0] - v[1])$
			for(j=0;j<d0;j++) {
				int m=userent[base0+j]&USER_MOVIEMASK;
				int r=(userent[base0+j]>>USER_LMOVIEMASK)&7;
 
				// for all softmax
				int rr;
				for(rr=0;rr<5;rr++) {
					//visbiases[(m*5+rr)] += EpsilonVB * ((posvisact[(m*5+rr)] - negvisact[(m*5+rr)]) + weightcost * visbiases[(m*5+rr)]);
//printf("vb: %f, pa: %f, na: %f\n", visbiases[(m*5+rr)], posvisact[(m*5+rr)], negvisact[(m*5+rr)]);
				}
				double vdelta = (r-negvissoftmax[m]);
				nrmse += vdelta * vdelta;
			}
			ntrain+=d0;

		//# At the end compute average of CDpos and CDneg by dividing them by number of data points.
		//# Compute CD = < Si.Sj >0  < Si.Sj >n = CDpos  CDneg
		//# Update weights and biases W = W + alpha*CD (biases are just weights to neurons that stay always 1.0)
		//# Compute some error function like sum of squared difference between Si in 1) and Si in 5)
		//e.g between data and reconstruction.

			if ( ((u+1) % 300) == 0 || (u+1) == NUSERS ) {
				int numcases = u % 300;
				numcases++;

				// Update weights and biases 
				for(m=0;m<NMOVIES;m++) {

					// for all hidden units h:
					for(h=0;h<TOTAL_FEATURES;h++) {
						// for all softmax
						int rr;
						for(rr=0;rr<5;rr++) {
							CDpos[(m*5+rr)*TOTAL_FEATURES+h] /= ((double)numcases);
							CDneg[(m*5+rr)*TOTAL_FEATURES+h] /= ((double)numcases);

							// W += epsilon * (h[0] * v[0]' - Q(h[1][.] = 1 | v[1]) * v[1]')
		//float preW = vishid[m][rr][h];
							vishid[m][rr][h] += EpsilonW * ((CDpos - CDneg) + weightcost * vishid[m][rr][h]);
		//printf("W: %f preW: %f, CDp: %f, CDn: %f, m: %d, r: %d, h: %d, nhp: %f, nvp: %f\n", vishid[m][rr][h], preW, CDpos, CDneg, m, rr, h,
		//neghidprobs[h],negvisprobs[m*5+rr]
		//);
						}
					}
				}

				// 4. now Si and Sj values can be used to compute (Si.Sj)0  here () means just values not average
				// 7. now use Si and Sj to compute (Si.Sj)1 (fig.3)
				// 8. repeating multiple times steps 5,6 and 7 compute (Si.Sj)n. Where n is small number and can 
				//    increase with learning steps to achieve better accuracy.

				
				// b += epsilon * (h[0] - Q(h[1][.] = 1 | v[1]))
				for(h=0;h<TOTAL_FEATURES;h++) {
					poshidact[h]  /= ((double)(numcases*ntrain*5));
					neghidact[h]  /= ((double)(numcases*ntrain*5));
					hidbiases[h] += EpsilonHB * ((poshidact[h] - neghidact[h]) + weightcost * hidbiases[h]);
		//printf("hb: %f, pa: %f, na: %f, d0:%d\n", hidbiases[h], poshidact[h], neghidact[h], d0);
				}

				// c += epsilon * (v[0] - v[1])$
				for(m=0;m<NMOVIES;m++) {

					// for all softmax
					int rr;
					for(rr=0;rr<5;rr++) {
						posvisact[(m*5+rr)] /= ((double)numcases);
						negvisact[(m*5+rr)] /= ((double)numcases);
						visbiases[(m*5+rr)] += EpsilonVB * ((posvisact[(m*5+rr)] - negvisact[(m*5+rr)]) + weightcost * visbiases[(m*5+rr)]);
		//printf("vb: %f, pa: %f, na: %f\n", visbiases[(m*5+rr)], posvisact[(m*5+rr)], negvisact[(m*5+rr)]);
					}
				}
				ZERO(CDpos);
				ZERO(CDneg);
				ZERO(poshidact);
				ZERO(neghidact);
				ZERO(posvisact);
				ZERO(negvisact);
			}

		}
		nrmse=sqrt(nrmse/ntrain);
		//prmse = sqrt(s/n);
		
		lg("%f\t%f\t%f\n",nrmse,prmse,(clock()-t0)/(double)CLOCKS_PER_SEC);
		//EpsilonW  *= 0.90;
		//EpsilonVB *= 0.90;
		//EpsilonHB *= 0.90;

	}
	
	/* Perform a final iteration in which the errors are clipped and stored */
	//removeUV();
	
	//if(save_model) {
		//dappend_bin(fnameV,sV,NMOVIES);
		//dappend_bin(fnameU,sU,NUSERS);
	//}
	
	return 1;
}
