//
//  vmo.cpp
//  
//
//  Created by Six Wang on 1/25/15.
//
//

#include "vmo.h"

vmo::vmo(){
	
	nStates = 0;
	thresh = 0.1;
	dim = 1;
}

void vmo::setup(int dim){
	
	this.dim = dim;
	
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

int vmo::lenCommonSfx(int ind, int symbol){
	
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
	
}

vector<float> vmo::_calculate_ir(){
	
}

void vmo::addState(vector<float> newData){
	// Add a new state to VMO
	
	// Update attributes
	sfx.push_back(0);
	rsfx.push_back(vector<int>(0));
	trn.push_back(vector<int>(0));
	lrs.push_back(0);
	obs.push_back(newData);
	
	nStates += 1;
	int ind = nStates -1; // Local index
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

void vmo::getIR(){
	
}

void vmo::print(string attr){
	
}

float vmo::findThreshold(vmo oracle, float start, float step, float end){
	
}

vmo vmo::buildOracle(vector<vector<float> > obs, float threshold){
	
}


