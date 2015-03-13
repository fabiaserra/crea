#pragma once
#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "irMarker.h"

//typedef vector<ofPolyline> Trajectories;

class Sequence{
    public:
        Sequence();

        void setup(const int numMarkers);
        void update();
        void record(const vector<irMarker>& markers);
        void load(const string path);
        void save(const string path);
        void draw();
        void drawTracking(int currentIdx);
        void drawPatterns(map<int, float>& currentPatterns);
        void drawPatternsSeparate(map<int, float>& currentPatterns);
        void drawSegments();

        void updateSegments(const vector< pair<float, float> >& segmentsPcts);

        void loadPatterns(vector< vector<ofPolyline> > patterns);
        void createPatterns(int nPatterns);

        void startRecording();
        void clearPlayback();

        ofPoint getCurrentPoint(int markerIdx);
        float getCurrentPercent(int currentIdx);

        int getNumMarkers();

        //--------------------------------------------------------------
        ofxXmlSettings xml;
        //--------------------------------------------------------------
        int maxPatternsWindow;
        //--------------------------------------------------------------
        vector<ofPolyline> markersPosition;                 // Markers positions through all the sequence
        //--------------------------------------------------------------
        vector< vector<ofPolyline> > patterns;              // Identified patterns from the sequence
        //--------------------------------------------------------------
        vector< vector<ofPolyline> > segments;              // Segments belonging to the different cues
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
        vector<ofPolyline> getSegment(const pair<float, float>& segmentPctRange);
        void updatePlayhead();
        size_t calcCurrentFrameIndex();
        int numMarkers;
};
