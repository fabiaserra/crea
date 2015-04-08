//
//  helper.h
//  zuzor
//
//  Created by Six Wang on 3/4/15.
//
//
#pragma once

#include "ofMain.h"
#include "vmo.h"
#include "Sequence.h"

float lowpass(float input, float past, float slide);

vector< vector<ofPolyline> > processPttr(vmo& oracle, vector< vector< float> > &obs, const vmo::pttr& pttrList, int nFeature, int dim);

map<int, float> getGestureUpdate(int ind, vmo& oracle, vmo::pttr& pttrList, Sequence& sequence);

vector< vector< float> > covarianceMat(vector< vector< float> > &input, int numMarker, int dim);

vector< vector< float> > centroidSplit(vector< vector< float> > &input, int numMarker, int dim);

vector<float> cov_cal(vector<float> &prevObs, vector<float> &obs, int numElements);

