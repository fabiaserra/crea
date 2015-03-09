//
//  helper.cpp
//  zuzor
//
//  Created by Six Wang on 3/4/15.
//
//

#include "helper.h"

float lowpass(float input, float past, float slide){
	return past + (input - past)/slide;
}

vector<vector<ofPolyline> > processPttr(vmo& oracle, vector< vector< float> > &obs,
										const vmo::pttr& pttrList, int nFeature, int dim){
	vector<vector<ofPolyline> > pattern(pttrList.size,
										vector<ofPolyline>(nFeature,
														   vector<ofPoint>(0)));
	vector1D pts(0);
	int len = 0;
	for (int i = 0; i < pttrList.size; i++) {
		pts = pttrList.sfxPts[i];
		len = pttrList.sfxLen[i];
		vector<ofPolyline> ges(nFeature, vector<ofPoint>(len, ofPoint(0.0,0.0)));
		pattern[i] = ges;
		for (int j = 0; j<pts.size(); j++) {
			int offset = pts[j]-len+1;
			for (int k = 0; k < len; k++) {
				oracle.pttrCat[offset+k].push_back(i);
				oracle.pttrInd[offset+k].push_back(k+1);
				
				for (int d = 0; d < nFeature; d++) {
					pattern[i][d][k].x = (pattern[i][d][k].x*float(j)/float(j+1))
					+ obs[offset+k][d*dim]/float(j+1);
					pattern[i][d][k].y = (pattern[i][d][k].y*float(j)/float(j+1))
					+ obs[offset+k][d*dim+1]/float(j+1);
				}
			}
		}
	}
	return pattern;
}

vector< vector< float> > covarianceMat(vector< vector< float> > &input, int numMarker, int dim){
	vector< vector<float> > tmpIn = input;
	vector< vector<float> >::iterator it = tmpIn.begin();
	
	tmpIn.insert(it, input[0]);
	vector< vector<float> > out(input.size(), vector< float>());
	int nElements = (numMarker*dim+1)*(numMarker*dim)/2;
	for (int i = 1; i < tmpIn.size(); i++) {
		out[i-1] = cov_cal(tmpIn[i-1], tmpIn[i], nElements);
	}
	return out;
}

vector<float> cov_cal(vector<float> &prevObs, vector<float> &obs, int numElements){
	vector<float> out(numElements, 0.0);
	float mean [obs.size()];
	for (int i = 0; i < obs.size(); i++) {
		mean[i] = (prevObs[i]+obs[i])/2.0;
	}
	int ind = 0;
	for (int i = 0; i < obs.size(); i++) {
		for (int j = 0; j<obs.size(); j++) {
			if (j <= i) {
				out[ind] = (prevObs[i]-mean[i])*(prevObs[j]-mean[j])+(obs[i]-mean[i])*(obs[j]-mean[j]);
				ind++;
			}
		}
	}
	return out;
}

// Not finished yet.
vector< vector< float> > centroidSplit(vector< vector< float> > &input, int numMarker, int dim){
	int len = input.size();
	vector< vector< float> > out(len, vector<float>(numMarker*dim, 0.0));
	for (int i = 0; i < len; i++) {
		for (int j = 0; j < numMarker; j++) {
			for (int d = 0; d < dim; d++) {
				out[i][d] = float(j)*out[i][d+j]/(1.0+j) + input[i][(numMarker-1)*j+d]/(1.0+j); // Centroid calculation
				// Unfinihsed here.
			}
		}
	}
}
