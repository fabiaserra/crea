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
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 

 vmo.h
 
 Original code by Greg Surges.
 Adapted by Cheng-i Wang on 1/25/15.
 -------------------------------------------------------------------------
 */


#ifndef ____vmo__
#define ____vmo__

#include <stdio.h>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <cmath>
//#include <armadillo>
#include <cfloat>

#include "ofMain.h"
#endif /* defined(____vmo__) */

const int INIT_VMO_SIZE = 2000;
const int INIT_VMO_K = 10;

using namespace std;
typedef vector<int> vector1D;
typedef vector<vector1D> vector2D;

class vmo{
public:
	vmo();
	// Main functions
	void setup(int dim, float threshold);
	void reset();
	void addState(vector<float> newData);
	
	// Getters
	int getK();
	float getTotalIR();
	float getIR();

	// Ultilities
	void print(string attr);

	// Attributes
	vector1D sfx;
	vector1D lrs;
	vector1D data;
//		vector1D maxLrs;

	vector2D trn;
	vector2D rsfx;
	vector2D latent;
	
	vector2D pttrCat; // Pattern label
	vector2D pttrInd; // Pattern sequence index

	vector<float> ir;
	vector<vector<float> > obs;

	int nStates;
	int dim;
	float thresh;

	// Static functions
	// Construction funcitons
	static float findThreshold(vector<vector<float> > obs, int dim,float start, float step, float end);
	static vmo buildOracle(vector<vector<float> > obs, float threshold);

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
	
	// Analysis functions
	static vmo::pttr findPttr(vmo oracle, int minLen);
	static vector<vector<ofVec2f> > processPttr(vmo oracle, vmo::pttr pttrList);
	static vmo::belief tracking_init(vmo::pttr pttrList, vmo oracle, vector<float> firstObs);
	static vmo::belief tracking(vmo::pttr pttrList, vmo oracle, vmo::belief prevState, vector<float>obs);
	
	// Interface with openFrameworks
//	static vector<vector<ofPoint> > pttr2Points(vmo::pttr pttrList);
	
	
private:
	// Helper functions
	int lenCommonSfx(int p1, int p2);
	float getDistance(vector<float> x, vector<float> y);
	vector<float> cumsum(vector<float> cw);
	vector<float> getDistArray(vector<float> x, vector<vector<float> > y);
	vector<vector<float> > trnIndexing(int n);
	vector2D encode();
	
};

