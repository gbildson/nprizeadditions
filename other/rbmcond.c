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
//#define epsilonw        0.002   // Learning rate for weights
//#define epsilonvb       0.002   // Learning rate for biases of visible units
//#define epsilonhb       0.002   // Learning rate for biases of hidden units
//#define weightcost      0.0002
#define epsilonw        0.00003   // Learning rate for weights
#define epsilond        0.000015   // Learning rate for weights
#define epsilonvb       0.00003   // Learning rate for biases of visible units
#define epsilonhb       0.00003   // Learning rate for biases of hidden units
#define weightcost      0.0002
//#define momentum        0.5  
#define momentum        0.9  
#define finalmomentum   0.9  
#define MAX_EPOCH       50


double vishid[NMOVIES][5][TOTAL_FEATURES];
double Dij[NMOVIES][TOTAL_FEATURES];
double visbiases[NMOVIES][5];
double hidbiases[TOTAL_FEATURES];
double CDpos[NMOVIES][5][TOTAL_FEATURES];
double CDneg[NMOVIES][5][TOTAL_FEATURES];
double CDinc[NMOVIES][5][TOTAL_FEATURES];
double DIJinc[NMOVIES][TOTAL_FEATURES];

double poshidprobs[TOTAL_FEATURES];
char   poshidstates[TOTAL_FEATURES]; 
double poshidact[TOTAL_FEATURES];
double neghidact[TOTAL_FEATURES];
double neghidprobs[TOTAL_FEATURES];
char   neghidstates[TOTAL_FEATURES]; 
double hidbiasinc[TOTAL_FEATURES];

double negvisprobs[NMOVIES][5];
char   negvissoftmax[NMOVIES]; 
double posvisact[NMOVIES][5];
double negvisact[NMOVIES][5];
double visbiasinc[NMOVIES][5];

unsigned int moviercount[5*NMOVIES];
unsigned int moviecount[NMOVIES];
unsigned int movieseencount[NMOVIES];


#define E2 (0.00020) // stop condition
#define E  (0.00002) // stop condition
int score_argv(char **argv) {return 0;}


#if 0
#define NFEATURES (50)
#define GLOBAL_MEAN (3.6033)
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
	}
	for(u=0;u<NUSERS;u++) {
		int base0=useridx[u][0];
		int d0=UNTRAIN(u);

		// For all rated movies
		for(j=0;j<d0;j++) {
			int m=userent[base0+j]&USER_MOVIEMASK;
			int r=(userent[base0+j]>>USER_LMOVIEMASK)&7;
			moviercount[m*5+r]++;
		}
	}
}

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
			//vishid[j][0][i] = 0.1 * randn(); // Normal Distribution
			//vishid[j][1][i] = 0.1 * randn(); // Normal Distribution
			//vishid[j][2][i] = 0.1 * randn(); // Normal Distribution
			//vishid[j][3][i] = 0.1 * randn(); // Normal Distribution
			//vishid[j][4][i] = 0.1 * randn(); // Normal Distribution
			vishid[j][0][i] = 0.035 * randn() - 0.0175; // Normal Distribution
			vishid[j][1][i] = 0.035 * randn() - 0.0175; // Normal Distribution
			vishid[j][2][i] = 0.035 * randn() - 0.0175; // Normal Distribution
			vishid[j][3][i] = 0.035 * randn() - 0.0175; // Normal Distribution
			vishid[j][4][i] = 0.035 * randn() - 0.0175; // Normal Distribution
			Dij[j][i] = 0.01 * randn() - 0.005; // Normal Distribution
	    }
	}

	/* Initial biases */
	for(i=0;i<TOTAL_FEATURES;i++) {
		hidbiases[i]=0.0;
	}
    //for (j=0; j<TOTAL_INPUTS; j++) 
    for (j=0; j<NMOVIES; j++) {
		unsigned int mtot = moviercount[j*5+0] + moviercount[j*5+1] + moviercount[j*5+2] + moviercount[j*5+3] + moviercount[j*5+4];
	    for (i=0; i<5; i++) {
		    visbiases[j][i] = log( ((double)moviercount[j*5+i]) / ((double) mtot) );
//printf("mrc: %d, mc %d, log:%f frac: %f\n", moviercount[j*5+i], moviecount[j] , log( moviercount[j*5+i] /(double) moviecount[j]), 
//(moviercount[j*5+i] /(double) moviecount[j]) );
		}
	}

	
	/* Optimize current feature */
	double nrmse=2., last_rmse=10.;
	double prmse = 0, last_prmse=0;
	double s;
	int n;
	int loopcount=0;
	double EpsilonW  = epsilonw;
	double EpsilonD  = epsilond;
	double EpsilonVB = epsilonvb;
	double EpsilonHB = epsilonhb;
	double Momentum  = momentum;
	ZERO(CDinc);
	ZERO(DIJinc);
	ZERO(visbiasinc);
	ZERO(hidbiasinc);
	int predictionPass;
	while( 1 )  {

	    if ( ((nrmse < (last_rmse-E) && prmse<last_prmse) || loopcount < 12) && loopcount < 80  )  {
			predictionPass = 0;
		} else {
			predictionPass = 1;
		}

		last_rmse=nrmse;
		last_prmse=prmse;
		clock_t t0=clock();
		loopcount++;
		int ntrain = 0;
		nrmse = 0.0;
		s = 0.0;
		n = 0;

		if ( loopcount > 5 )
			Momentum = finalmomentum;

		//* CDpos =0, CDneg=0 (matrices)
		ZERO(CDpos);
		ZERO(CDneg);
		ZERO(poshidact);
		ZERO(neghidact);
		ZERO(posvisact);
		ZERO(negvisact);
		ZERO(moviecount);
		ZERO(movieseencount);

		int u,m, f;
		for(u=0;u<NUSERS;u++) {

			//* CDpos =0, CDneg=0 (matrices)
			ZERO(negvisprobs);

		    //* perform steps 1 to 8

			int base0=useridx[u][0];
			int d0=UNTRAIN(u);
			int dall=UNALL(u);

			// For all rated movies, accumulate contributions to hidden units
			double sumW[TOTAL_FEATURES];
			ZERO(sumW);
			for(j=0;j<dall;j++) {
				int m=userent[base0+j]&USER_MOVIEMASK;

   				// 1. get one data point from data set.
   				// 2. use values of this data point to set state of visible neurons Si
				int r=(userent[base0+j]>>USER_LMOVIEMASK)&7;
 
				if ( j < d0 ) {
					moviecount[m]++;
					// for all hidden units h:
				    // Add to hidden probabilities based on softmax value of a rating 
					for(h=0;h<TOTAL_FEATURES;h++) {
						// sum_j(W[i][j] * v[0][j]))
						sumW[h]  += vishid[m][r][h];
					}
				} 
				
				// Add to hidden probabilities based on existence of a rating
				movieseencount[m]++;
				for(h=0;h<TOTAL_FEATURES;h++) {
					// sum_j(Dij * rij)
					sumW[h]  += Dij[m][h];
				}
				
			}

			// Sample the hidden units state after computing probabilities
			for(h=0;h<TOTAL_FEATURES;h++) {

   			    // 3. compute Sj for each hidden neuron based on formula above and states of visible neurons Si
			    // poshidprobs[h] = 1./(1 + exp(-V*vishid - hidbiases);
				// compute Q(h[0][i] = 1 | v[0]) # for binomial units, sigmoid(b[i] + sum_j(W[i][j] * v[0][j]))
				poshidprobs[h]  = 1.0/(1.0 + exp(-sumW[h] - hidbiases[h]));

				// sample h[0][i] from Q(h[0][i] = 1 | v[0])
				if  ( poshidprobs[h] >  (rand()/(double)(RAND_MAX)) ) {
					poshidstates[h]=1;
					poshidact[h] += 1.0;
				} else {
					poshidstates[h]=0;
				}
				//poshidact[h] += poshidprobs[h];
			}

			// 5. on visible neurons compute Si using the Sj computed in step3. This is known as reconstruction
			// for all visible units j:
			int r;
			int count = d0;
			count += useridx[u][1];  // too compute probe errors
			if ( predictionPass )
				count = dall;
			for(j=0;j<count;j++) {
				int m=userent[base0+j]&USER_MOVIEMASK;
				for(h=0;h<TOTAL_FEATURES;h++) {
			    	if ( poshidstates[h] == 1 ) {
						for(r=0;r<5;r++) {
//printf("phs: %d, vishid: %f\n", poshidstates[h] , vishid[m][r][h]);
							//negvisprobs[m*5+r]  += poshidstates[h] * vishid[m][r][h];
							negvisprobs[m][r]  += vishid[m][r][h];
						}
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
				negvisprobs[m][0]  = 1./(1 + exp(-negvisprobs[m][0] - visbiases[m][0]));
				negvisprobs[m][1]  = 1./(1 + exp(-negvisprobs[m][1] - visbiases[m][1]));
				negvisprobs[m][2]  = 1./(1 + exp(-negvisprobs[m][2] - visbiases[m][2]));
				negvisprobs[m][3]  = 1./(1 + exp(-negvisprobs[m][3] - visbiases[m][3]));
				negvisprobs[m][4]  = 1./(1 + exp(-negvisprobs[m][4] - visbiases[m][4]));
//printf("RAW nvp: %f, nvp: %f, nvp: %f, nvp: %f, nvp: %f, m: %d\n", 
//negvisprobs[m*5+0],
//negvisprobs[m*5+1],
//negvisprobs[m*5+2],
//negvisprobs[m*5+3],
//negvisprobs[m*5+4],m);

				// Normalize probabilities
				double tsum  = 
				  negvisprobs[m][0] +
				  negvisprobs[m][1] +
				  negvisprobs[m][2] +
				  negvisprobs[m][3] +
				  negvisprobs[m][4];
//printf("BEFORE nvp: %f, nvp: %f, nvp: %f, nvp: %f, nvp: %f, m: %d, tsum: %f\n", 
//negvisprobs[m*5+0],
//negvisprobs[m*5+1],
//negvisprobs[m*5+2],
//negvisprobs[m*5+3],
//negvisprobs[m*5+4],m, tsum);
				if ( tsum != 0 ) {
					negvisprobs[m][0]  /= tsum;
					negvisprobs[m][1]  /= tsum;
					negvisprobs[m][2]  /= tsum;
					negvisprobs[m][3]  /= tsum;
					negvisprobs[m][4]  /= tsum;
				}
//printf("nvp: %f, nvp: %f, nvp: %f, nvp: %f, nvp: %f, m: %d, tsum: %f\n", 
//negvisprobs[m*5+0],
//negvisprobs[m*5+1],
//negvisprobs[m*5+2],
//negvisprobs[m*5+3],
//negvisprobs[m*5+4],m, tsum);

        	    // sample v[1][j] from P(v[1][j] = 1 | h[0])
				double randval = (rand()/(double)(RAND_MAX));
				if ( (randval -= negvisprobs[m][0]) <= 0.0 )
				    negvissoftmax[m] = 0;
				else if ( (randval -= negvisprobs[m][1]) <= 0.0 )
				    negvissoftmax[m] = 1;
				else if ( (randval -= negvisprobs[m][2]) <= 0.0 )
				    negvissoftmax[m] = 2;
				else if ( (randval -= negvisprobs[m][3]) <= 0.0 )
				    negvissoftmax[m] = 3;
				else //if ( (randval -= negvisprobs[m][4]) <= 0.0 )
				    negvissoftmax[m] = 4;
				//negvisact[m*5+0] += negvisprobs[m*5+0];
				//negvisact[m*5+1] += negvisprobs[m*5+1];
				//negvisact[m*5+2] += negvisprobs[m*5+2];
				//negvisact[m*5+3] += negvisprobs[m*5+3];
				//negvisact[m*5+4] += negvisprobs[m*5+4];

				// if in training data then train on it
				if ( j < d0 )  
				    negvisact[m][negvissoftmax[m]] += 1.0;
			}


			// 6. compute state of hidden neurons Sj again using Si from 5 step.
			// For all rated movies accumulate contributions to hidden units from sampled visible units
			ZERO(sumW);
			for(j=0;j<dall;j++) {
				int m=userent[base0+j]&USER_MOVIEMASK;
 
				if ( j < d0 ) {
					// for all hidden units h:
					for(h=0;h<TOTAL_FEATURES;h++) {
						sumW[h]  += vishid[m][negvissoftmax[m]][h];
						//sumW[h]  += vishid[m][0][h] * negvisprobs[m*5+0];
						//sumW[h]  += vishid[m][1][h] * negvisprobs[m*5+1];
						//sumW[h]  += vishid[m][2][h] * negvisprobs[m*5+2];
						//sumW[h]  += vishid[m][3][h] * negvisprobs[m*5+3];
						//sumW[h]  += vishid[m][4][h] * negvisprobs[m*5+4];
					}
				}

				// Add to hidden probabilities based on existence of a rating
				for(h=0;h<TOTAL_FEATURES;h++) {
					// sum_j(Dij * rij)
					sumW[h]  += Dij[m][h];
				}
			}
			// for all hidden units h:
			for(h=0;h<TOTAL_FEATURES;h++) {
			    // compute Q(h[1][i] = 1 | v[1]) # for binomial units, sigmoid(b[i] + sum_j(W[i][j] * v[1][j]))
				neghidprobs[h]  = 1./(1 + exp(-sumW[h] - hidbiases[h]));

			    // Experimentally sample the hidden units state again
				if  ( neghidprobs[h] >  (rand()/(double)(RAND_MAX)) ) {
					neghidstates[h]=1;
					neghidact[h] += 1.0;
				} else {
					neghidstates[h]=0;
				}

				//neghidact[h] += neghidprobs[h];
			}

			if ( predictionPass ) {
				int base0=useridx[u][0];
				int d012=UNALL(u);

				for(i=0; i<d012;i++) {
					int m=userent[base0+i]&USER_MOVIEMASK;
					int r=(userent[base0+i]>>USER_LMOVIEMASK)&7;
					double expectedV = negvisprobs[m][1] + 2.0 * negvisprobs[m][2] + 3.0 * negvisprobs[m][3] + 4.0 * negvisprobs[m][4];
					double vdelta = (((double)r)-expectedV);
					err[base0+i] = vdelta;
				}

			} else { // if ( !predictionPass ) 

				// Update weights and biases and sum up the error
				for(j=0;j<d0;j++) {
					int m=userent[base0+j]&USER_MOVIEMASK;
					int r=(userent[base0+j]>>USER_LMOVIEMASK)&7;
					posvisact[m][r] += 1.0;
	 
					// for all hidden units h:
					for(h=0;h<TOTAL_FEATURES;h++) {
						if ( poshidstates[h] == 1 ) {
							// 4. now Si and Sj values can be used to compute (Si.Sj)0  here () means just values not average
							//* accumulate CDpos = CDpos + (Si.Sj)0
							CDpos[m][r][h] += 1.0;
						}
						// 7. now use Si and Sj to compute (Si.Sj)1 (fig.3)
						//CDneg[(m*5+negvissoftmax[m])*TOTAL_FEATURES+h] += neghidprobs[h];
						CDneg[m][negvissoftmax[m]][h] += (double)neghidstates[h];
	//TODO - This is experimental!!!!!!!
					}
				}

				// 8. repeating multiple times steps 5,6 and 7 compute (Si.Sj)n. Where n is small number and can 
				//    increase with learning steps to achieve better accuracy.


				// Compute error
				for(j=0;j<d0;j++) {
					int m=userent[base0+j]&USER_MOVIEMASK;
					int r=(userent[base0+j]>>USER_LMOVIEMASK)&7;
	 
					//# Compute some error function like sum of squared difference between Si in 1) and Si in 5)
					double expectedV = negvisprobs[m][1] + 2.0 * negvisprobs[m][2] + 3.0 * negvisprobs[m][3] + 4.0 * negvisprobs[m][4];
					double vdelta = (((double)r)-expectedV);
					//double vdelta = (r-negvissoftmax[m]);
					nrmse += (vdelta * vdelta);
				}
				ntrain+=d0;

				// Sum up probe rmse
				int base=useridx[u][0];
				for(i=1;i<2;i++) base+=useridx[u][i];
				int d=useridx[u][2];
				for(i=0; i<d;i++) {
					int m=userent[base+i]&USER_MOVIEMASK;
					int r=(userent[base+i]>>USER_LMOVIEMASK)&7;
					//# Compute some error function like sum of squared difference between Si in 1) and Si in 5)
					double expectedV = negvisprobs[m][1] + 2.0 * negvisprobs[m][2] + 3.0 * negvisprobs[m][3] + 4.0 * negvisprobs[m][4];
					double vdelta = (((double)r)-expectedV);
					s+=vdelta*vdelta;
				}
				n+=d;

				int bsize = 500;
				if ( ((u+1) % bsize) == 0 || (u+1) == NUSERS ) {
					int numcases = u % bsize;
					numcases++;
					//if ( numcases != bsize ) printf("u: %d, numcases: %d\n", u, numcases);

					// Update weights and biases 
					for(m=0;m<NMOVIES;m++) {
						if ( moviecount[m] == 0 ) continue;

						// for all hidden units h:
						for(h=0;h<TOTAL_FEATURES;h++) {
							// for all softmax
							int rr;
							for(rr=0;rr<5;rr++) {
								//# At the end compute average of CDpos and CDneg by dividing them by number of data points.
								//# Compute CD = < Si.Sj >0  < Si.Sj >n = CDpos  CDneg
								CDpos[m][rr][h] /= ((double)moviecount[m]);
								double CDp = CDpos[m][rr][h];
								CDneg[m][rr][h] /= ((double)moviecount[m]);
								double CDn = CDneg[m][rr][h];
								CDinc[m][rr][h] = Momentum * CDinc[m][rr][h] + EpsilonW * ((CDp - CDn) + weightcost * vishid[m][rr][h]);

								// W += epsilon * (h[0] * v[0]' - Q(h[1][.] = 1 | v[1]) * v[1]')
								//# Update weights and biases W = W + alpha*CD (biases are just weights to neurons that stay always 1.0)
								//e.g between data and reconstruction.
	//double preW = vishid[m][rr][h];
								//vishid[m][rr][h] += EpsilonW * ((CDp - CDn) + weightcost * vishid[m][rr][h]);
								vishid[m][rr][h] += CDinc[m][rr][h];
	//printf("W: %f preW: %f, CDp: %f, CDn: %f, m: %d, r: %d, h: %d, nhp: %f, nvp: %f\n", vishid[m][rr][h], preW, CDp, CDn, m, rr, h,
	//neghidprobs[h],negvisprobs[m*5+rr]
	//);
								//if ( CDp != 0.0 || CDn != 0.0 ) {
								//}
							}
						}

						// Update visible softmax biases
						// c += epsilon * (v[0] - v[1])$
						// for all softmax
						int rr;
						for(rr=0;rr<5;rr++) {
							posvisact[m][rr] /= ((double)moviecount[m]);
							negvisact[m][rr] /= ((double)moviecount[m]);
							visbiasinc[m][rr] = Momentum * visbiasinc[m][rr] + EpsilonVB * ((posvisact[m][rr] - negvisact[m][rr]) + weightcost * visbiases[m][rr]);
							//visbiases[m][rr] += EpsilonVB * ((posvisact[m][rr] - negvisact[m][rr]) + weightcost * visbiases[m][rr]);
							visbiases[m][rr]  += visbiasinc[m][rr];
	//printf("vb: %f, pa: %f, na: %f\n", visbiases[(m*5+rr)], posvisact[(m*5+rr)], negvisact[(m*5+rr)]);
							//if ( posvisact[m][rr] != 0.0 || negvisact[m][rr] != 0.0 ) {
							//}
						}
					}

					// Update hidden biases 
					// b += epsilon * (h[0] - Q(h[1][.] = 1 | v[1]))
					for(h=0;h<TOTAL_FEATURES;h++) {
						poshidact[h]  /= ((double)(numcases));
						neghidact[h]  /= ((double)(numcases));
						hidbiasinc[h] = Momentum * hidbiasinc[h] + EpsilonHB * ((poshidact[h] - neghidact[h]) + weightcost * hidbiases[h]);
						//hidbiases[h] += EpsilonHB * ((poshidact[h] - neghidact[h]) + weightcost * hidbiases[h]);
						hidbiases[h]  += hidbiasinc[h];
			//printf("hb: %f, pa: %f, na: %f, d0:%d\n", hidbiases[h], poshidact[h], neghidact[h], d0);
					}

					// Update all DIJ factors
					for(m=0;m<NMOVIES;m++) {
						if ( movieseencount[m] == 0 ) continue;
						// for all hidden units h:
						for(h=0;h<TOTAL_FEATURES;h++) {
							// Update conditional Dij factors
						    DIJinc[m][h] = Momentum * DIJinc[m][h] + EpsilonD * ((poshidact[h] - neghidact[h]) + weightcost * Dij[m][h]);
						    Dij[m][h]   += DIJinc[m][h];
						}
					}
					
					ZERO(CDpos);
					ZERO(CDneg);
					ZERO(poshidact);
					ZERO(neghidact);
					ZERO(posvisact);
					ZERO(negvisact);
					ZERO(moviecount);
					ZERO(movieseencount);
				}
			}

		}
		if ( predictionPass ) 
			return 1;

		nrmse=sqrt(nrmse/ntrain);
		prmse = sqrt(s/n);
		
		lg("%f\t%f\t%f\n",nrmse,prmse,(clock()-t0)/(double)CLOCKS_PER_SEC);
		//EpsilonW  *= 0.90;
		//EpsilonD  *= 0.90;
		//EpsilonVB *= 0.90;
		//EpsilonHB *= 0.90;
		//printf("dd: %d %d %d %d %d\n", dd[0], dd[1], dd[2], dd[3], dd[4]);
	}
	
	/* Perform a final iteration in which the errors are clipped and stored */
	//removeUV();
	
	//if(save_model) {
		//dappend_bin(fnameV,sV,NMOVIES);
		//dappend_bin(fnameU,sU,NUSERS);
	//}
	
	return 1;
}
