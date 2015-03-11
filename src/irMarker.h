#pragma once
#include "ofMain.h"
#include "ofxCv.h"

class irMarker : public ofxCv::RectFollower{
    public:
        irMarker();

        void setup(const cv::Rect& track);
        void update(const cv::Rect& track);
        void updateLabels(vector<unsigned int> deadLabels, vector<unsigned int> currentLabels);
        void draw();
        void drawPath();
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
