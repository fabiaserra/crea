#pragma once
#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "Marker.h"

class Sequence{
	public:
		Sequence();

		void setup();
		void update(vector<Marker>& markers);

		void startRecording();
		void stopRecording();
		void load(const string path);
		void save();

		// void eraseGestureFile();

		//--------------------------------------------------------------
		ofxXmlSettings xml;

		bool recording;

		ofPolyline line;
		vector< vector<ofPoint> > sequence;
		int frame_counter;
};
