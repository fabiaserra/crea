/*
 -------------------------------------------------------------------------
 vmo - Variable Markov Oracle
 implements the Variable Markov Oracle for time series analysis and
 generation

 copyright 2015 greg surges & Cheng-i Wang

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 vmo.h

 Original code by Greg Surges.
 Adapted by Cheng-i Wang on 1/25/15.
 -------------------------------------------------------------------------
 */
#pragma once
#ifndef ____vmo__
#define ____vmo__

//#include <stdio.h>
//#include <vector>
//#include <algorithm>
//#include <stdlib.h>
//#include <fstream>
//#include <iostream>
//#include <string>
//#include <sstream>
//#include <cmath>
//#include <armadillo>
//#include <cfloat>

#include "ofMain.h"
#endif /* defined(____vmo__) */

const int INIT_VMO_SIZE = 2000;
const int INIT_VMO_K = 10;

using namespace std;
typedef vector<int> vector1D;
typedef vector<vector1D> vector2D;

class vmo{
public:
    class pttr{
        public:
            pttr();
            int size = 0;
            vector2D sfxPts;
            vector1D sfxLen;
	};
	class belief{
        public:
            belief();
            int K;
            int currentIdx;
            vector1D path;
            vector<float> cost;
	};
	vmo();
	// Main functions
	void setup(int numElement, float threshold);
	void reset();
	void addState(vector<float>& newData);

	// Getters
	int getK();
	float getTotalIR();
	float getIR();

	// Ultilities
	void print(string attr);
	int* getGestureCat(int ind);
	int* getGestureInd(int ind);
	map<int, float> getGestureUpdate(int ind, vmo::pttr& pttrList);

	// Attributes
	vector1D sfx;
	vector1D lrs;
	vector1D data;

	vector2D trn;
	vector2D rsfx;
	vector2D latent;

	vector2D pttrCat; // Pattern label
	vector2D pttrInd; // Pattern sequence index

	vector<float> ir;
	vector<vector<float> > obs;

	int nStates;
//	int dimFeature;
//	int numFeature;
	int nElement;
	float thresh;

	// Static functions
	// Construction funcitons
//	static float findThreshold(vector<vector<float> > &obs, int dim, int num, float start, float step, float end);
//	static vmo buildOracle(vector<vector<float> > &obs, int dim, int num, float threshold);

	static float findThreshold(vector<vector<float> > &obs, int numElement, float start, float step, float end);
	static vmo buildOracle(vector<vector<float> > &obs, int numElement, float threshold);
	// Analysis functions
	static vmo::pttr findPttr(const vmo& oracle, int minLen);
	static vmo::belief &tracking_init(vmo& oracle, vmo::belief &bf,
									  const vmo::pttr& pttrList,
									  vector<float> &firstObs);
	static vmo::belief &tracking(vmo& oracle, vmo::belief& prevState,
								 const vmo::pttr& pttrList,
								 vector<float> &obs, float decay);

private:
    // Helper functions
    static float getDistance(vector<float> &x, vector<float> &y);
    static vector<float> getDistArray(vector<float> &x, vector<vector<float> > &y);

    int lenCommonSfx(int p1, int p2);
    vector<float> cumsum(vector<float> &cw);
    vector2D encode();
};
