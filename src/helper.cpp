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


vector<vector<ofPolyline> > processPttr(vmo& oracle, const vmo::pttr& pttrList){
	
	vector<vector<ofPolyline> > pattern(pttrList.size,
										vector<ofPolyline>(oracle.numFeature,
														   vector<ofPoint>(0)));
	vector1D pts(0);
	int len = 0;
	for (int i = 0; i < pttrList.size; i++) {
		pts = pttrList.sfxPts[i];
		len = pttrList.sfxLen[i];
		vector<ofPolyline> ges(oracle.numFeature, vector<ofPoint>(len, ofPoint(0.0,0.0)));
		pattern[i] = ges;
		for (int j = 0; j<pts.size(); j++) {
			int offset = pts[j]-len+1;
			for (int k = 0; k < len; k++) {
				oracle.pttrCat[offset+k].push_back(i);
				oracle.pttrInd[offset+k].push_back(k+1);
				
				for (int d = 0; d < oracle.numFeature; d++) {
					pattern[i][d][k].x = (pattern[i][d][k].x*float(j)/float(j+1))
					+ oracle.obs[offset+k][d*oracle.dimFeature]/float(j+1);
					pattern[i][d][k].y = (pattern[i][d][k].y*float(j)/float(j+1))
					+ oracle.obs[offset+k][d*oracle.dimFeature+1]/float(j+1);
				}
			}
		}
	}
	return pattern;
}
