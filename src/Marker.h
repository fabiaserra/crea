#pragma once
#include "ofMain.h"
#include "ofxCv.h"

class Marker : public ofxCv::RectFollower{
	public:
		Marker();

		void setup(const cv::Rect& track);
		void update(const cv::Rect& track);
		void update(vector<unsigned int> deadLabels, vector<unsigned int> currentLabels);
		void draw();
		void kill();
		//--------------------------------------------------------------
		ofPoint currentPos;
		ofPoint previousPos;
		ofPoint smoothPos;
		ofPoint velocity;
		//--------------------------------------------------------------
		ofColor color;
		ofPolyline all;
		//--------------------------------------------------------------
		float startedDying;
		float dyingTime;
		float timeDead;
		//--------------------------------------------------------------
		bool hasDisappeared;
		//--------------------------------------------------------------
		float bornRate;
};
