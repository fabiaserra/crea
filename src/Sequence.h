#pragma once
#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "irMarker.h"

class Sequence{
    public:
        Sequence();

        void setup(int nMarkers);
        void update();
        void record(vector<irMarker>& markers);
        void load(const string path);
        void save(const string path);
        void draw();
        void drawPatterns(map<int, float> currentPatterns);

//        void setPatterns(vector< vector<ofPolyline> > patterns);

        void startRecording();
        void clearPlayback();

        //--------------------------------------------------------------
        ofxXmlSettings xml;
        //--------------------------------------------------------------
        size_t maxMarkers;
        //--------------------------------------------------------------
        vector<ofPolyline> markersPosition;                 // Markers positions through all the sequence
        vector<ofPolyline> markersPastPoints;               // Percentage completion fragment of the sequence

//        map<int, vector<ofPolyline> > markersPosition;      // Markers positions through all the sequence with label
        // Other ideas
        // vector< vector<ofPoint> > markersPosition;
        // vector< vector<Marker> > markersPosition;
        // vector< Frame > frames;
        //--------------------------------------------------------------
        vector< vector<ofPolyline> > patterns;              // identified patterns from the sequence
        vector< vector<ofPolyline> > patternsPastPoints;    // Percentage completion fragments of the patterns
        //--------------------------------------------------------------
        int frame_counter;
        float duration;
        size_t numFrames;
        //--------------------------------------------------------------
        float playhead; // 0 ~ 1
        float elapsedTime;
        //--------------------------------------------------------------
        ofTrueTypeFont  verdana;

    protected:
        void drawPattern(int patternPosition, int patternIndex, float percent, bool highlight);
        void updatePlayhead();
        size_t calcCurrentFrameIndex();
};
