#pragma once
#include "ofMain.h"
#include "ofxCv.h"
#include "ParticleSystem.h"


class Marker : public ofxCv::RectFollower
{
    public:
        Marker();

        void setup(const cv::Rect& track);
        void update(const cv::Rect& track);
        void draw();
        void kill();

        ofPoint currentPos;
        ofPoint previousPos;
        ofPoint smoothPos;
        ofPoint velocity;

        ofColor color;
        ofPolyline all;

        float startedDying;
};
