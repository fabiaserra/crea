#pragma once
#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "Marker.h"

class Sequence{
	public:
		Sequence();

		void setup(int nMarkers);
		void update(vector<Marker>& markers);
		void load(const string path);
		void save(const string path);
        void draw(float percent);
        void drawPatterns(float percent);

        void startRecording();
		void stopRecording();
		// void deleteRecording();

		//--------------------------------------------------------------
		ofxXmlSettings xml;
        //--------------------------------------------------------------
		bool recording;
		size_t nMarkers;
        //--------------------------------------------------------------
        vector<ofPolyline> frames;
        // Different ideas
//		vector< vector<ofPoint> > frames;
//		vector< vector<Marker> > frames;
//		vector< Frame > frames;
        //--------------------------------------------------------------
		ofPolyline line;
        vector< vector<ofPolyline> > patterns;
        vector<ofPolyline> previousPoints;
		//--------------------------------------------------------------
		int frame_counter;
		float duration;
};
