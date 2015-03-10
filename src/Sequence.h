#pragma once
#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "irMarker.h"

//typedef vector<ofPolyline> Trajectories;

class Sequence{
    public:
        Sequence();

        void setup(const int nMarkers);
        void update();
        void record(const vector<irMarker>& markers);
        void load(const string path);
        void save(const string path);
        void draw();
        void drawPatterns(map<int, float>& currentPatterns);
        void drawPatternsInSequence(map<int, float>& currentPatterns);
        void drawSequenceTracking(float percent);
		void drawSequenceTracking2(int currentIdx);
        void drawSequenceSegments();

        void updateSequenceSegments(const vector< pair<float, float> >& sequencePcts);

        void loadPatterns(vector< vector<ofPolyline> > patterns);
        void createPatterns(int nPatterns);

        void startRecording();
        void clearPlayback();

        ofPoint getCurrentPoint(int markerIdx);

        //--------------------------------------------------------------
        ofxXmlSettings xml;
        //--------------------------------------------------------------
        size_t maxMarkers;
        //--------------------------------------------------------------
        int maxPatternsWindow;
        //--------------------------------------------------------------
        vector<ofPolyline> markersPosition;                 // Markers positions through all the sequence
        //--------------------------------------------------------------
        vector< vector<ofPolyline> > patterns;              // Identified patterns from the sequence
        //--------------------------------------------------------------
        vector< vector<ofPolyline> > sequenceSegments;      // Sequence segments belonging to the different cues
        //--------------------------------------------------------------
        string filename;
        float duration;
        size_t numFrames;
        //--------------------------------------------------------------
        float playhead; // 0 ~ 1
        float elapsedTime;
        //--------------------------------------------------------------
        ofTrueTypeFont  verdana;

    protected:
        void drawPattern(const int patternPosition, const int patternIdx, float percent, const bool highlight);
        vector<ofPolyline> getSequenceSegment(const pair<float, float>& sequenceSegmentPct);
        void updatePlayhead();
        size_t calcCurrentFrameIndex();
};
