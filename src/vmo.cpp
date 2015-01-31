//
//  vmo.cpp
//  
//
//  Created by Six Wang on 1/25/15.
//
//

#include "vmo.h"

vmo::vmo(int dim = 1, float threshold = 0.0){
	
	nStates = 1;
	this.dim = dim;
	this.thresh = threshold;
	// Foward link vector
	vector1D zeroStateTrn;
	trn.clear();
	trn.reserve(INIT_VMO_SIZE);
	trn.push_back(zeroStateTrn);
	
	// Suffix link vector
	sfx.clear();
	sfx.assign(INIT_VMO_SIZE, 0);
	sfx[0] = -1;
	
	// Reverse suffix link vector
	vector1D zeroStateRsfx;
	rsfx.clear();
	rsfx.reserve(INIT_VMO_SIZE);
	rsfx.push_back(zeroStateRsfx);
	
	// Longest repeated suffix
	lrs.clear();
	lrs.assign(INIT_VMO_SIZE, 0);
	lrs[0] = 0;
	
	// Data vector - symbolized token for time series
	data.clear();
	data.assign(INIT_VMO_SIZE, 0);
	data[0] = -1; //MARK: Might be problematic to initialize 0th state`s symbol as -1.
	
	// State cluster vector
	vector1D zeroStateLatent;
	latent.clear();
	latent.reserve(INIT_VMO_K);
	latent.push_back(zeroStateLatent);
	
	// Observation vector
	vector<float> zeroStateObs(this->dim, 0.0);
	obs.clear();
	obs.reserve(INIT_VMO_SIZE);
	obs.push_back(zeroStateObs);
	
	// IR (information rate) vector
	ir.clear();
	ir.assign(2000, 0.0);
	
	// Maximum LRS
	//	maxLrs.clear();
	//	maxLrs.assign(INIT_VMO_SIZE, 0);
	//	maxLrs[0] = 0;
}

void vmo::setup(int dim = 1, float threshold = 0.0){
	
	nStates = 0;
	this.dim = dim;
	this.thresh = threshold;
	// Foward link vector
	vector1D zeroStateTrn;
	trn.clear();
	trn.reserve(INIT_VMO_SIZE);
	trn.push_back(zeroStateTrn);
	
	// Suffix link vector
	sfx.clear();
	sfx.assign(INIT_VMO_SIZE, 0);
	sfx[0] = -1;
	
	// Reverse suffix link vector
	vector1D zeroStateRsfx;
	rsfx.clear();
	rsfx.reserve(INIT_VMO_SIZE);
	rsfx.push_back(zeroStateRsfx);
	
	// Longest repeated suffix
	lrs.clear();
	lrs.assign(INIT_VMO_SIZE, 0);
	lrs[0] = 0;
	
	// Data vector - symbolized token for time series
	data.clear();
	data.assign(INIT_VMO_SIZE, 0);
	data[0] = -1; //MARK: Might be problematic to initialize 0th state`s symbol as -1.
	
	// State cluster vector
	vector1D zeroStateLatent;
	latent.clear();
	latent.reserve(INIT_VMO_K);
	latent.push_back(zeroStateLatent);
	
	// Observation vector
	vector<float> zeroStateObs(this->dim, 0.0);
	obs.clear();
	obs.reserve(INIT_VMO_SIZE);
	obs.push_back(zeroStateObs);
	
	// IR (information rate) vector
	ir.clear();
	ir.assign(2000, 0.0);
	
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

float vmo::getDistance(vector<float> x, vector<float> y){
	float d = 0.0;
	for (int i = 0; i < x.size(); i++) {
		d += (x[i]-y[i]) * (x[i]-y[i]);
	}
	d = sqrt(d);
	return d;
}

vector<float> vmo::getDistArray(vector<float> x, vector<vector<float> > y){
	vector<float> dvec(x.size(), 0.0);
	for (int i = 0; i < y.size(); i++) {
		dvec[i] = getDistance(x, y[i]);
	}
	return dvec;
}

vector<vector<float> > vmo::trnIndexing(int n){
	vector<vector<float> > temp(trn[n].size(), vector<float>(dim, 0.0));
	for (int i = 0; i < trn[n].size(); i++) {
		temp[i] = obs[trn[n][i]];
	}
	return temp;
}

vector2D vmo::encode(){
	vector2D code;
	int j = 0;
	int i = j;
	while (j < nStates - 1) {
		while (i < nStates - 1 && lrs[i+1]>=(i-j+1)) {
			i++;
		}
		vector1D cw;
		if (i == j) {
			i++;
			cw.push_back(0);
			cw.push_back(i);
		}else{
			cw.push_back(i-j);
			cw.push_back(sfx[i]-i+j+1);
		}
		code.push_back(cw);
	}
	return code;
}

void vmo::addState(vector<float> newData){
	// Add a new state to VMO
	
	// Update attributes
	sfx.push_back(0);
	rsfx.push_back(vector<int>(0));
	trn.push_back(vector<int>(0));
	lrs.push_back(0);
	obs.push_back(newData);
	
	nStates++;
	int ind = nStates - 1; // Local index
	trn[ind-1].push_back(ind);
	
	int k = sfx[ind-1];
	int piOne = ind - 1;
	
	int sfxCandidate = 0;
	vector1D trnList;
	vector<float> trnVec;
	
	while (k >= 0) {
		trnList.clear();
		trnVec.clear();
		
		vector<float> dvec = getDistArray(newData, trnIndexing(k));
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

void vmo::getK(){
	
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

vector<float> vmo::getIR(){
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
	
	for (int i = 0; i < nStates-1; i++) {
		h0[i] = log2f(h0[i]+FLT_MIN);
		h1[i] = log2f(h1[i]+FLT_MIN)/block[i];
		float tmpIR = h1[i] - h0[i];
		ir[i] = (tmpIR > 0) ? tmpIR:0.0;
	}
	return ir;
}

float vmo::getTotalIR(){
	vector<float> irVec = getIR();
	float irSum = 0.0;
	for (vector<float>::iterator j = irVec.begin(); j != irVec.end(); ++j) {
		irSum += *j;
	}
	return irSum;
}

void vmo::print(string attr){
	
}

float vmo::findThreshold(vector<vector<float> > obs, int dim = 1,float start, float step, float end){
	float t = start;
	float ir = 0.0;
	while (start <= end) {
		tmpVmo = buildOracle(obs, dim, start);
		float tmpIr = tmpVmo.getTotalIR();
		if (tmpIr >= ir) {
			ir = tmpIr;
			t = start;
		}
		start += step;
	}
	return t;
}

vmo vmo::buildOracle(vector<vector<float> > obs, int dim = 1, float threshold = 0.0){
	oracle = vmo(dim, threshold);
	
	for (int i = 0; i<obs.size(); i++) {
		oracle.addState(obs[i]);
	}
	return oracle;
}


