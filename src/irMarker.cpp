#include "irMarker.h"

using namespace ofxCv;
using namespace cv;

irMarker::irMarker(){
    startedDying    = 0;
    dyingTime       = 3;
    bornRate        = 3;
    hasDisappeared  = false;
}

void irMarker::setup(const cv::Rect& track){
    color.setHsb(ofRandom(0, 255), 255, 255);
    currentPos = toOf(track).getCenter();
    smoothPos = currentPos;
    previousPos = currentPos;
}

void irMarker::update(const cv::Rect& track){
    currentPos = toOf(track).getCenter();
    smoothPos.interpolate(currentPos, .5);
    all.addVertex(smoothPos);
    velocity = smoothPos - previousPos;
    previousPos = smoothPos;
}

void irMarker::updateLabels(vector<unsigned int> deadLabels, vector<unsigned int> currentLabels){
    // Labels that have disappeared but can appear again
    for(unsigned int i = 0; i < deadLabels.size(); i++){
        if(deadLabels[i] == label){
            hasDisappeared = true;
        }
    }

    // Labels that are currently being tracked
    for(unsigned int i = 0; i < currentLabels.size(); i++){
        if(currentLabels[i] == label){
            hasDisappeared = false;
        }
    }
}

void irMarker::draw(){
    ofPushStyle();
    float size = 16;
    ofSetColor(255);
    if(startedDying){
        ofSetColor(ofColor::red);
        size = ofMap(ofGetElapsedTimef() - startedDying, 0, dyingTime, size, 0, true);
    }
    ofNoFill();
    ofCircle(currentPos, size);
    ofSetColor(color);
    all.draw();
    ofSetColor(255);
    ofDrawBitmapString(ofToString(label), currentPos);
    ofPopStyle();
}

void irMarker::kill(){
    float currentTime = ofGetElapsedTimef();
    if(startedDying == 0){
        startedDying = currentTime;
    }
    else if((currentTime - startedDying) > dyingTime){
        dead = true;
    }
}
