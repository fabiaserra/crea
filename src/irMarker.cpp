#include "irMarker.h"

using namespace ofxCv;
using namespace cv;

irMarker::irMarker(){
    startedDying    = 0;
    dyingTime       = 3;
    hasDisappeared  = true;
}

void irMarker::setup(const cv::Rect& track){
    color.setHsb(ofRandom(0, 255), 255, 255);
    currentPos = toOf(track).getCenter();
    smoothPos = currentPos;
    previousPos = currentPos;
    all.curveTo(smoothPos); // necessary duplicate first point for control point
    hasDisappeared  = false;
}

void irMarker::update(const cv::Rect& track){
    currentPos = toOf(track).getCenter();
    smoothPos.interpolate(currentPos, .5);
    all.curveTo(smoothPos);
    velocity = smoothPos - previousPos;
    previousPos = smoothPos;
}

void irMarker::updateLabels(vector<unsigned int> deadLabels, vector<unsigned int> currentLabels){
    // Labels that have disappeared but can appear again
    if(find(deadLabels.begin(), deadLabels.end(), label) != deadLabels.end()) hasDisappeared = true;

    // Labels that are currently being tracked
    if(find(currentLabels.begin(), currentLabels.end(), label) != currentLabels.end()) hasDisappeared = false;
}

void irMarker::draw(){
    ofPushStyle();
    float size = 16;
    ofFill();
    ofSetLineWidth(3);
    if(startedDying){
        ofNoFill();
        size = ofMap(ofGetElapsedTimef() - startedDying, 0, dyingTime, size, 0, true);
    }
    ofSetColor(color);
    ofCircle(currentPos, size);
    ofSetColor(color.getInverted());
    ofDrawBitmapString(ofToString(label), currentPos.x-2, currentPos.y+2);
    ofPopStyle();
}

void irMarker::drawPath(){
    ofPushStyle();
    ofSetColor(color);
    ofSetLineWidth(1.5);
    all.draw();
    ofPopStyle();
}

void irMarker::kill(){
    float currentTime = ofGetElapsedTimef();
    if(startedDying == 0){
        startedDying = currentTime;
        all.curveTo(smoothPos);  // necessary duplicate last point for control point
    }
    else if((currentTime - startedDying) > dyingTime){
        dead = true;
    }
}

void irMarker::clearPath(){
    all.clear();
}
