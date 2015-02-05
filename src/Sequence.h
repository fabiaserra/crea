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
        void drawPattern(int patternPosition, int patternIndex, float percent, bool highlight);

        void startRecording();
		void stopRecording();
		// void deleteRecording();

		//--------------------------------------------------------------
		ofxXmlSettings xml;
        //--------------------------------------------------------------
		bool recording;
		bool sequenceLoaded;
		size_t nMarkers;
        //--------------------------------------------------------------
        vector<ofPolyline> markersPosition;                 // Markers positions through all the sequence
        vector<ofPolyline> markersPastPoints;               // Percentage completion fragment of the sequence

        // Different ideas
//		vector< vector<ofPoint> > frames;
//		vector< vector<Marker> > frames;
//		vector< Frame > frames;
        //--------------------------------------------------------------
        vector< vector<ofPolyline> > patterns;              // identified patterns from the sequence
        vector< vector<ofPolyline> > patternsPastPoints;    // Percentage completion fragments of the patterns
		//--------------------------------------------------------------
		int frame_counter;
		float duration;
		size_t numFrames;
		//--------------------------------------------------------------
		ofTrueTypeFont	verdana;
};
