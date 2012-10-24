//=============================================================================
//
// SVD Sample Code
//
// Copyright (C) 2007 Timely Development (www.timelydevelopment.com)
//
// Special thanks to Simon Funk and others from the Netflix Prize contest 
// for providing pseudo-code and tuning hints.
//
// Feel free to use this code as you wish as long as you include 
// these notices and attribution. 
//
// Also, if you have alternative types of algorithms for accomplishing 
// the same goal and would like to contribute, please share them as well :)
//
// STANDARD DISCLAIMER:
//
// - THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY
// - OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT
// - LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR
// - FITNESS FOR A PARTICULAR PURPOSE.
//
//=============================================================================

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <math.h>
//#include <tchar.h>
#include <map>
using namespace std;

//===================================================================
//
// Constants and Type Declarations
//
//===================================================================
#define TRAINING_PATH   "netflixtraining_set*.txt"
#define TRAINING_FILE   "netflixtraining_set\%s"
#define FEATURE_FILE    "netflixfeatures.txt"
#define TEST_PATH       "netflix\%s"
#define PREDICTION_FILE "netflixprediction.txt"

#define MAX_RATINGS     100480508     // Ratings in entire training set (+1)
#define MAX_CUSTOMERS   480190        // Customers in the entire training set (+1)
#define MAX_MOVIES      17771         // Movies in the entire training set (+1)
#define MAX_FEATURES    64            // Number of features to use 
#define MIN_EPOCHS      120           // Minimum number of epochs per feature
#define MAX_EPOCHS      200           // Max epochs per feature

#define MIN_IMPROVEMENT 0.0001        // Minimum improvement required to continue current feature
#define INIT            0.1           // Initialization value for features
#define LRATE           0.001         // Learning rate parameter
#define K               0.015         // Regularization parameter used to minimize over-fitting

typedef unsigned char BYTE;
typedef map<int, int> IdMap;
typedef IdMap::iterator IdItr;

struct Movie
{
    int         RatingCount;
    int         RatingSum;
    double      RatingAvg;            
    double      PseudoAvg;            // Weighted average used to deal with small movie counts 
};

struct Customer
{
    int         CustomerId;
    int         RatingCount;
    int         RatingSum;
};

struct Data
{
    int         CustId;
    short       MovieId;
    BYTE        Rating;
    float       Cache;
};

class Engine 
{
private:
    int             m_nRatingCount;                                 // Current number of loaded ratings
    Data            m_aRatings[MAX_RATINGS];                        // Array of ratings data
    Movie           m_aMovies[MAX_MOVIES];                          // Array of movie metrics
    Customer        m_aCustomers[MAX_CUSTOMERS];                    // Array of customer metrics
    float           m_aMovieFeatures[MAX_FEATURES][MAX_MOVIES];     // Array of features by movie (using floats to save space)
    float           m_aCustFeatures[MAX_FEATURES][MAX_CUSTOMERS];   // Array of features by customer (using floats to save space)
    IdMap           m_mCustIds;                                     // Map for one time translation of ids to compact array index

    inline double   PredictRating(short movieId, int custId, int feature, float cache, bool bTrailing=true);
    inline double   PredictRating(short movieId, int custId);

    bool            ReadNumber(char* pwzBufferIn, int nLength, int &nPosition, char* pwzBufferOut);
    bool            ParseInt(char* pwzBuffer, int nLength, int &nPosition, int& nValue);
    bool            ParseFloat(char* pwzBuffer, int nLength, int &nPosition, float& fValue);

public:
    Engine(void);
    ~Engine(void) { };

    void            CalcMetrics();
    void            CalcFeatures();
    void            LoadHistory();
    void            ProcessTest(char* pwzFile);
    void            ProcessFile(char* pwzFile);
};


//===================================================================
//
// Program Main
//
//===================================================================
int main(int argc, CHAR* argv[])
{
    Engine* engine = new Engine();

    engine->LoadHistory();
    engine->CalcMetrics();
    engine->CalcFeatures();
    engine->ProcessTest("qualifying.txt");

    printf(" Done ");
    getchar();
    
    return 0;
}


//===================================================================
//
// Engine Class 
//
//===================================================================

//-------------------------------------------------------------------
// Initialization
//-------------------------------------------------------------------

Engine::Engine(void)
{
    m_nRatingCount = 0;

    for (int f=0; f<MAX_FEATURES; f++)
    {
        for (int i=0; i<MAX_MOVIES; i++) m_aMovieFeatures[f][i] = (float)INIT;
        for (int i=0; i<MAX_CUSTOMERS; i++) m_aCustFeatures[f][i] = (float)INIT;
    }
}

//-------------------------------------------------------------------
// Calculations - This Paragraph contains all of the relevant code
//-------------------------------------------------------------------

//
// CalcMetrics
// - Loop through the history and pre-calculate metrics used in the training 
// - Also re-number the customer id's to fit in a fixed array
//
void Engine::CalcMetrics()
{
    int i, cid;
    IdItr itr;

    printf(" Calculating intermediate metrics ");

    // Process each row in the training set
    for (i=0; i<m_nRatingCount; i++)
    {
        Data* rating = m_aRatings + i;

        // Increment movie stats
        m_aMovies[rating->MovieId].RatingCount++;
        m_aMovies[rating->MovieId].RatingSum += rating->Rating;
        
        // Add customers (using a map to re-number id's to array indexes) 
        itr = m_mCustIds.find(rating->CustId); 
        if (itr == m_mCustIds.end())
        {
            cid = 1 + (int)m_mCustIds.size();

            // Reserve new id and add lookup
            m_mCustIds[rating->CustId] = cid;

            // Store off old sparse id for later
            m_aCustomers[cid].CustomerId = rating->CustId;

            // Init vars to zero
            m_aCustomers[cid].RatingCount = 0;
            m_aCustomers[cid].RatingSum = 0;
        }
        else
        {
            cid = itr->second;
        }

        // Swap sparse id for compact one
        rating->CustId = cid;

        m_aCustomers[cid].RatingCount++;
        m_aCustomers[cid].RatingSum += rating->Rating;
    }

    // Do a follow-up loop to calc movie averages
    for (i=0; i<MAX_MOVIES; i++)
    {
        Movie* movie = m_aMovies+i;
        movie->RatingAvg = movie->RatingSum / (1.0 * movie->RatingCount);
        movie->PseudoAvg = (3.23 * 25 + movie->RatingSum) / (25.0 + movie->RatingCount);
    }
}

//
// CalcFeatures
// - Iteratively train each feature on the entire data set
// - Once sufficient progress has been made, move on
//
void Engine::CalcFeatures()
{
    int f, e, i, custId, cnt = 0;
    Data* rating;
    double err, p, sq, rmse_last, rmse = 2.0;
    short movieId;
    float cf, mf;

    for (f=0; f<MAX_FEATURES; f++)
    {
        printf(" --- Calculating feature: %d --- ", f);

        // Keep looping until you have passed a minimum number 
        // of epochs or have stopped making significant progress 
        for (e=0; (e < MIN_EPOCHS) || (rmse <= rmse_last - MIN_IMPROVEMENT); e++)
        {
            cnt++;
            sq = 0;
            rmse_last = rmse;

            for (i=0; i<m_nRatingCount; i++)
            {
                rating = m_aRatings + i;
                movieId = rating->MovieId;
                custId = rating->CustId;

                // Predict rating and calc error
                p = PredictRating(movieId, custId, f, rating->Cache, true);
                err = (1.0 * rating->Rating - p);
                sq += err*err;
                
                // Cache off old feature values
                cf = m_aCustFeatures[f][custId];
                mf = m_aMovieFeatures[f][movieId];

                // Cross-train the features
                m_aCustFeatures[f][custId] += (float)(LRATE * (err * mf - K * cf));
                m_aMovieFeatures[f][movieId] += (float)(LRATE * (err * cf - K * mf));
            }
            
            rmse = sqrt(sq/m_nRatingCount);
                  
            printf("     <set x='%d' y='%f' /> ",cnt,rmse);
        }

        // Cache off old predictions
        for (i=0; i<m_nRatingCount; i++)
        {
            rating = m_aRatings + i;
            rating->Cache = (float)PredictRating(rating->MovieId, rating->CustId, f, rating->Cache, false);
        }            
    }
}

//
// PredictRating
// - During training there is no need to loop through all of the features
// - Use a cache for the leading features and do a quick calculation for the trailing
// - The trailing can be optionally removed when calculating a new cache value
//
double Engine::PredictRating(short movieId, int custId, int feature, float cache, bool bTrailing)
{
    // Get cached value for old features or default to an average
    double sum = (cache > 0) ? cache : 1; //m_aMovies[movieId].PseudoAvg; 

    // Add contribution of current feature
    sum += m_aMovieFeatures[feature][movieId] * m_aCustFeatures[feature][custId];
    if (sum > 5) sum = 5;
    if (sum < 1) sum = 1;

    // Add up trailing defaults values
    if (bTrailing)
    {
        sum += (MAX_FEATURES-feature-1) * (INIT * INIT);
        if (sum > 5) sum = 5;
        if (sum < 1) sum = 1;
    }

    return sum;
}

//
// PredictRating
// - This version is used for calculating the final results
// - It loops through the entire list of finished features
//
double Engine::PredictRating(short movieId, int custId)
{
    double sum = 1; //m_aMovies[movieId].PseudoAvg;

    for (int f=0; f<MAX_FEATURES; f++) 
    {
        sum += m_aMovieFeatures[f][movieId] * m_aCustFeatures[f][custId];
        if (sum > 5) sum = 5;
        if (sum < 1) sum = 1;
    }

    return sum;
}

//-------------------------------------------------------------------
// Data Loading / Saving
//-------------------------------------------------------------------

//
// LoadHistory
// - Loop through all of the files in the training directory
//
void Engine::LoadHistory()
{
    //WIN32_FIND_DATA FindFileData;
    //HANDLE hFind;
    //bool bContinue = true;
    //int count = 0; // TEST

    // Loop through all of the files in the training directory
    //hFind = FindFirstFile(TRAINING_PATH, &FindFileData);
    //if (hFind == INVALID_HANDLE_VALUE) return;
    
    //while (bContinue) 
    //{
        //this->ProcessFile(FindFileData.cFileName);
        //bContinue = (FindNextFile(hFind, &FindFileData) != 0);
//
        ////if (++count > 999) break; // TEST: Uncomment to only test with the first X movies
//    } 

    //FindClose(hFind);
}

//
// ProcessFile
// - Load a history file in the format:
//
//   <MovieId>:
//   <CustomerId>,<Rating>
//   <CustomerId>,<Rating>
//   ...
void Engine::ProcessFile(char_t* pwzFile)
{
    FILE *stream;
    char pwzBuffer[1000];
    wsprintf(pwzBuffer,TRAINING_FILE,pwzFile);
    int custId, movieId, rating, pos = 0;
    
    wprintf("Processing file: %s
", pwzBuffer);

    if (_wfopen_s(&stream, pwzBuffer, "r") != 0) return;

    // First line is the movie id
    fgetws(pwzBuffer, 1000, stream);
    ParseInt(pwzBuffer, (int)wcslen(pwzBuffer), pos, movieId);
    m_aMovies[movieId].RatingCount = 0;
    m_aMovies[movieId].RatingSum = 0;

    // Get all remaining rows
    fgetws(pwzBuffer, 1000, stream);
    while ( !feof( stream ) )
    {
        pos = 0;
        ParseInt(pwzBuffer, (int)wcslen(pwzBuffer), pos, custId);
        ParseInt(pwzBuffer, (int)wcslen(pwzBuffer), pos, rating);
        
        m_aRatings[m_nRatingCount].MovieId = (short)movieId;
        m_aRatings[m_nRatingCount].CustId = custId;
        m_aRatings[m_nRatingCount].Rating = (BYTE)rating;
        m_aRatings[m_nRatingCount].Cache = 0;
        m_nRatingCount++;

        fgetws(pwzBuffer, 1000, stream);
    }

    // Cleanup
    fclose( stream );
}

//
// ProcessTest
// - Load a sample set in the following format
//
//   <Movie1Id>:
//   <CustomerId>
//   <CustomerId>
//   ...
//   <Movie2Id>:
//   <CustomerId>
//
// - And write results:
//
//   <Movie1Id>:
//   <Rating>
//   <Raing>
//   ...
void Engine::ProcessTest(char* pwzFile)
{
    FILE *streamIn, *streamOut;
    char pwzBuffer[1000];
    int custId, movieId, pos = 0;
    double rating;
    bool bMovieRow;

    sprintf(pwzBuffer, TEST_PATH, pwzFile);
    printf(" Processing test: %s ", pwzBuffer);

    if (_wfopen_s(&streamIn, pwzBuffer, "r") != 0) return;
    if (_wfopen_s(&streamOut, PREDICTION_FILE, "w") != 0) return;

    fgetws(pwzBuffer, 1000, streamIn);
    while ( !feof( streamIn ) )
    {
        bMovieRow = false;
        for (int i=0; i<(int)wcslen(pwzBuffer); i++)
        {
            bMovieRow |= (pwzBuffer[i] == 58); 
        }

        pos = 0;
        if (bMovieRow)
        {
            ParseInt(pwzBuffer, (int)wcslen(pwzBuffer), pos, movieId);

            // Write same row to results
            fputws(pwzBuffer,streamOut); 
        }
        else
        {
            ParseInt(pwzBuffer, (int)wcslen(pwzBuffer), pos, custId);
            custId = m_mCustIds[custId];
            rating = PredictRating(movieId, custId);

            // Write predicted value
            sprintf(pwzBuffer,1000,"%5.3f ",rating);
            fputws(pwzBuffer,streamOut);
        }

        //wprintf("Got Line: %d %d %d ", movieId, custId, rating);
        fgetws(pwzBuffer, 1000, streamIn);
    }

    // Cleanup
    fclose( streamIn );
    fclose( streamOut );
}

//-------------------------------------------------------------------
// Helper Functions
//-------------------------------------------------------------------
bool Engine::ReadNumber(char* pwzBufferIn, int nLength, int &nPosition, char* pwzBufferOut)
{
    int count = 0;
    int start = nPosition;
    char wc = 0;

    // Find start of number
    while (start < nLength)
    {
        wc = pwzBufferIn[start];
        if ((wc >= 48 && wc <= 57) || (wc == 45)) break;
        start++;
    }

    // Copy each character into the output buffer
    nPosition = start;
    while (nPosition < nLength && ((wc >= 48 && wc <= 57) || wc == 69  || wc == 101 || wc == 45 || wc == 46))
    {
        pwzBufferOut[count++] = wc;
        wc = pwzBufferIn[++nPosition];
    }

    // Null terminate and return
    pwzBufferOut[count] = 0;
    return (count > 0);
}

bool Engine::ParseFloat(char* pwzBuffer, int nLength, int &nPosition, float& fValue)
{
    char pwzNumber[20];
    bool bResult = ReadNumber(pwzBuffer, nLength, nPosition, pwzNumber);
    fValue = (bResult) ? (float)_wtof(pwzNumber) : 0;
    return false;
}

bool Engine::ParseInt(char* pwzBuffer, int nLength, int &nPosition, int& nValue)
{
    char pwzNumber[20];
    bool bResult = ReadNumber(pwzBuffer, nLength, nPosition, pwzNumber);
    nValue = (bResult) ? _wtoi(pwzNumber) : 0;
    return bResult;
}
    
