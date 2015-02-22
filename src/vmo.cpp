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


 vmo.cpp

 Original code by Greg Surges.
 Adapted by Cheng-i Wang on 1/25/15.
 -------------------------------------------------------------------------
 */

#include "vmo.h"

vmo::pttr::pttr(){
    size = 0;
	
	vector2D tmpPts(0);
	sfxPts = tmpPts;
//	sfxPts.reserve(INIT_VMO_SIZE);
	
	vector1D tmpLen(0);
	sfxLen = tmpLen;
//	sfxLen.reserve(INIT_VMO_SIZE);
}

vmo::belief::belief(){
	K = 0;
	currentIdx = -1;
	
	vector1D tmpPath(0);
	path = tmpPath;
	
	vector<float> tmpCost(0);
	cost = tmpCost;
}

vmo::vmo(){
}

void vmo::setup(int dim = 1, int num = 1, float threshold = 0.0){

	nStates = 1;
	this->dimFeature = dim;
	this->numFeature = num;
	this->thresh = threshold;

	// Suffix link vector
	sfx.clear();
	sfx.reserve(INIT_VMO_SIZE);
	sfx.push_back(-1);

	// Longest repeated suffix
	lrs.clear();
	lrs.reserve(INIT_VMO_SIZE);
	lrs.push_back(0);

	// Data vector - symbolized token for time series
	data.clear();
	data.reserve(INIT_VMO_SIZE);
	data.push_back(-1); //MARK: Might be problematic to initialize 0th state`s symbol as -1.

	// Foward link vector
	vector1D zeroStateTrn(0);
	trn.clear();
	trn.reserve(INIT_VMO_SIZE);
	trn.push_back(zeroStateTrn);

	// Reverse suffix link vector
	vector1D zeroStateRsfx;
	rsfx.clear();
	rsfx.reserve(INIT_VMO_SIZE);
	rsfx.push_back(zeroStateRsfx);

	// State cluster vector
//	vector1D zeroStateLatent;
	latent.clear();
	latent.reserve(INIT_VMO_K);
//	latent.push_back(zeroStateLatent);

	// Pattern label vector
	vector1D zeroStatePttrCat;
	pttrCat.clear();
	pttrCat.reserve(INIT_VMO_SIZE);
	pttrCat.push_back(zeroStatePttrCat);

	// Pattern sequence index vector
	vector1D zeroStatePttrInd;
	pttrInd.clear();
	pttrInd.reserve(INIT_VMO_SIZE);
	pttrInd.push_back(zeroStatePttrInd);

	// Observation vector
	vector<float> zeroStateObs(dimFeature*numFeature, 0.0);
	obs.clear();
	obs.reserve(INIT_VMO_SIZE);
	obs.push_back(zeroStateObs);

	// IR (information rate) vector
	ir.clear();
	ir.reserve(INIT_VMO_SIZE);
	ir.push_back(0.0);

	// Maximum LRS
//	maxLrs.clear();
//	maxLrs.assign(INIT_VMO_SIZE, 0);
//	maxLrs[0] = 0;
}

void vmo::reset(){
	nStates = 0;
	thresh = 0.1;

	trn.clear();
	sfx.clear();
	rsfx.clear();
	lrs.clear();
	data.clear();
	latent.clear();
	obs.clear();
	ir.clear();
//	maxLrs.clear();
}

int vmo::lenCommonSfx(int p1, int p2){
	if (p2 == sfx[p1]) {
		return lrs[p1];
	}else{
		while (sfx[p2] != sfx[p1] && p2 != 0) {
			p2 = sfx[p2];
		}
	}
	return min(lrs[p2], lrs[p1]);
}

float vmo::getDistance(vector<float> &x, vector<float> &y){
	float d = 0.0;
	for (int i = 0; i < x.size(); i++) {
		d += (x[i]-y[i]) * (x[i]-y[i]);
	}
	d = sqrt(d);
	return d;
}

vector<float> vmo::getDistArray(vector<float> &x, vector< vector<float> > &y){
	vector<float> dvec(y.size(), 0.0);
	for (int i = 0; i < y.size(); i++) {
		dvec[i] = vmo::getDistance(x, y[i]);
	}
	return dvec;
}

vector2D vmo::encode(){
	vector2D code;
	code.clear();
	code.reserve(nStates);
	int j = 0;
	int i = j;
	while (j < nStates - 1) {
		while (i < nStates - 1 && lrs[i+1]>=(i-j+1)) {
			i++;
		}
		vector1D cw(2);
		cw.clear();
		if (i == j) {
			i++;
			cw.push_back(0);
			cw.push_back(i);
		}else{
			cw.push_back(i-j);
			cw.push_back(sfx[i]-i+j+1);
		}
		j = i;
		code.push_back(cw);
	}
	return code;
}

void vmo::addState(vector<float>& newData){
	// Add a new state to VMO

	// Update attributes
	sfx.push_back(0);
	rsfx.push_back(vector1D(0));
	trn.push_back(vector1D(0));
	lrs.push_back(0);
	obs.push_back(newData);

	pttrCat.push_back(vector1D(0));
	pttrInd.push_back(vector1D(0));

	nStates++;
	int ind = nStates - 1; // Local index
	trn[ind-1].push_back(ind);

	int k = sfx[ind-1];
	int piOne = ind-1;

	int sfxCandidate = 0;

	while (k >= 0) {
		vector1D trnList(0);
		vector<float> trnVec(0);
		vector<vector<float> > tmp(trn[k].size(), vector<float>(dimFeature*numFeature, 0.0));
		for (int i = 0; i < trn[k].size(); i++) {
			tmp[i] = obs[trn[k][i]];
		}
        vector<float> dvec = vmo::getDistArray(newData, tmp);
		for (int i = 0; i < dvec.size(); i++) {
			if (dvec[i] < thresh) {
				trnList.push_back(i);
				trnVec.push_back(dvec[i]);
			}
		}
		if (trnList.size() == 0) {
			trn[k].push_back(ind);
			piOne = k;
			k = sfx[k];
		}else{
			int argmin = distance(trnVec.begin(), min_element(trnVec.begin(), trnVec.end()));
			sfxCandidate = trn[k][trnList[argmin]];
			break;
		}
	}

	if (k == -1) {
		sfx[ind] = 0;
		lrs[ind] = 0;
		latent.push_back(vector1D(1,ind));
		data.push_back(latent.size()-1);
	}else{
		sfx[ind] = sfxCandidate;
		lrs[ind] = lenCommonSfx(piOne, sfx[ind]-1) + 1;
		latent[data[sfx[ind]]].push_back(ind);
		data.push_back(data[sfx[ind]]);
	}

	rsfx[sfx[ind]].push_back(ind);
}

vector<float> vmo::cumsum(vector<float> cw){
	vector<float> out;
	float sum = 0;
	for(int i = 0; i < cw.size(); i++){
		sum += cw[i];
		out.push_back(sum);
	}
	return out;
}

float vmo::getIR(){
	vector2D code = encode();
	vector<float> cw0 (nStates-1, 0.0);
	vector<float> cw1 (nStates-1, 0.0);
	vector<float> block (nStates-1, 0.0);
	vector<float> ir (nStates-1, 0.0);

	int j = 0;
	for (int i = 0; i<code.size(); i++) {
		if (code[i][0] == 0) {
			cw0[j] = 1.0;
			cw1[j] = 1.0;
			block[j] = 1.0;
 			j++;
		}else{
			int len = code[i][0];
			cw1[j] = 1.0;
			fill(block.begin()+j, block.begin()+j+len, float(len));
			j+=len;
		}
	}

	vector<float> h0 = cumsum(cw0);
	vector<float> h1 = cumsum(cw1);

	float irSum = 0.0;
	for (int i = 0; i < nStates-1; i++) {
		h0[i] = log2f(h0[i]+FLT_MIN);
		h1[i] = log2f(h1[i]+FLT_MIN)/block[i];
		float tmpIR = h1[i] - h0[i];
		ir[i] = (tmpIR > 0) ? tmpIR:0.0;
		this->ir.push_back(ir[i]);
		irSum += ir[i];
	}
	return irSum;
}

void vmo::print(string attr){

}

float vmo::findThreshold(vector<vector<float> > &obs, int dim = 1, int num = 1,float start = 0.0, float step = 0.01, float end = 2.0){
	float t = start;
	float ir = 0.0;
	while (start <= end) {
		vmo tmpVmo = buildOracle(obs, dim, num, start);
		float tmpIr = tmpVmo.getIR();
		if (tmpIr >= ir) {
			ir = tmpIr;
			t = start;
		}
		start += step;
	}
	return t;
}

vmo vmo::buildOracle(vector<vector<float> > &obs, int dim = 1, int num = 1, float threshold = 0.0){
	vmo oracle = vmo();
	oracle.setup(dim, num, threshold);

	for (int i = 0; i<obs.size(); i++) {
		oracle.addState(obs[i]);
	}
	return oracle;
}

vmo::pttr vmo::findPttr(const vmo& oracle, int minLen = 0){
	vmo::pttr pttrList = vmo::pttr();
	int preSfx = -1;

	for (int i = oracle.nStates-1; i > minLen; i--) {
		int s = oracle.sfx[i];
		vector1D r = oracle.rsfx[i];
		bool pttrFound = false;

		if (
			(s != 0) &&
			((i - oracle.lrs[i]+1) > s) &&
			oracle.lrs[i] > minLen) {
			for (int j = 0; j < pttrList.size; j++) {
				vector1D tmp(0);
				for (int k = 0; k < pttrList.sfxPts[j].size(); k++) {
					if ((pttrList.sfxPts[j][k]-pttrList.sfxLen[j]) < i &&
						pttrList.sfxPts[j][k] > i) {
						tmp.push_back(pttrList.sfxPts[j][k]);
					}
				}
				if (tmp.size() == 0) {
					if (find(pttrList.sfxPts[j].begin(), pttrList.sfxPts[j].end(), s)!=pttrList.sfxPts[j].end()) {
						pttrList.sfxPts[j].push_back(i);
						int lrsLen = min(pttrList.sfxLen[j], oracle.lrs[i]);
						pttrFound = true;
					}else{
						pttrFound = false;
						break;
					}
				}
			}
			if (
				((preSfx - s) != 1) &&
				(!pttrFound)) {
				if (r.size() != 0) {
					r.push_back(i);
					r.push_back(s);
					vector1D lrsVec(0);
					for (int k = r.size()-1; k > -1; k--) {
						lrsVec.push_back(oracle.lrs[r[k]]);
					}
					int len = *min_element(lrsVec.begin(), lrsVec.end());
					if (len > minLen) {
						pttrList.sfxPts.push_back(r);
						pttrList.sfxLen.push_back(len);
					}
				}else{
					vector1D pts(0);
					pts.push_back(i);
					pts.push_back(s);
					pttrList.sfxPts.push_back(pts);
					pttrList.sfxLen.push_back(oracle.lrs[i]);
				}
				pttrList.size = pttrList.sfxLen.size();
			}
			preSfx = s;

		}else{
			preSfx = -1;
		}
	}
	return pttrList;
}

vector<vector<ofPolyline> > vmo::processPttr(vmo& oracle, const vmo::pttr& pttrList){

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
		int cat = i+1;
		for (int j = 0; j<pts.size(); j++) {
			int offset = pts[j]-len+1;
			for (int k = 0; k < len; k++) {
				oracle.pttrCat[offset+k].push_back(cat);
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


vmo::belief &vmo::tracking_init(vmo &oracle, vmo::belief &bf,
								const vmo::pttr &pttrList, vector<float> &firstObs){

	bf.K = oracle.latent.size();
	bf.path.assign(bf.K, 0);
	bf.cost.assign(bf.K, 0.0);

	int firstIdx = -1;
	float firstCost = FLT_MAX;
	for (int k = 0; k < bf.K; k++) {
		float minD = FLT_MAX;
		int ind = -1;
		float d = 0.0;
		for (int i = 0; i < oracle.latent[k].size(); i++) {
			int idx = oracle.latent[k][i];
            d = getDistance(firstObs, oracle.obs[idx]);
			if (d < minD) {
				minD = d;
				ind = idx;
				bf.path[k] = ind;
				bf.cost[k] = minD;
			}
		}
		if (minD < firstCost) {
			firstIdx = ind;
			firstCost = minD;
		}
	}
	bf.currentIdx = firstIdx;
	return bf;
}

vmo::belief &vmo::tracking(vmo &oracle,
						   const vmo::pttr &pttrList,
						   vmo::belief &prevBf, vector<float> &obs){
	/*
	 Real-time tracking function for VMO, not optimized yet.
	 */
//	vector1D stateCache;
//	vector<float> distCache;

	int tempIdx = -1;
	float tempCost = FLT_MAX;
	for (int k = 0; k < prevBf.K; k++) {
		float minD = FLT_MAX;
		int ind = -1;

		// Self-transition
		int selfTrn = oracle.data[prevBf.path[k]];
		for (int i = 0; i < oracle.latent[selfTrn].size(); i++) {
			float d = getDistance(obs, oracle.obs[oracle.latent[selfTrn][i]]);
			if (d < minD) {
				minD = d;
				ind = oracle.latent[selfTrn][i];
				prevBf.path[k] = ind;
				prevBf.cost[k] = minD;
			}
		}

		// Possible states from forward links
		int sym = -1;
		for (int j = 0; j < oracle.trn[prevBf.path[k]].size(); j++) {
			sym = oracle.data[oracle.trn[prevBf.path[k]][j]];
			float d = 0.0;
			for (int i = 0; i < oracle.latent[sym].size(); i++) {
				d = getDistance(obs, oracle.obs[oracle.latent[sym][i]]);
				if (d < minD) {
					minD = d;
					ind = oracle.latent[sym][i];
					prevBf.path[k] = ind;
					prevBf.cost[k] = minD;
				}
			}
		}
		if (minD < tempCost) {
			tempCost = minD;
			tempIdx = ind;
		}
	}
	prevBf.currentIdx = tempIdx;
	return prevBf;
}

map<int, float> vmo::getGestureUpdate(int ind, vmo::pttr& pttrList){
	map<int, float> out;
	if (pttrCat[ind].size() == 0 || ind == -1) {
		out[-1] = 0.0;
	}else{
		float idx;
		float len;
		for (int i = 0; i < pttrCat[ind].size(); i++) {
			idx = float(pttrInd[ind][i]);
			len = float(pttrList.sfxLen[pttrCat[ind][i]-1]);
			out[pttrCat[ind][i]] = ofMap(idx, 0.0, len, 0.0, 1.0);
		}
	}
	return out;
}

