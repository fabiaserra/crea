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
    if(find(deadLabels.begin(), deadLabels.end(), label) != deadLabels.end()) hasDisappeared = true;

    // Labels that are currently being tracked
    if(find(currentLabels.begin(), currentLabels.end(), label) != currentLabels.end()) hasDisappeared = false;
}

void irMarker::draw(){
//    ofPushStyle();
//    float size = 16;
//    ofSetColor(255);
//    if(startedDying){
//        ofSetColor(ofColor::red);
//        size = ofMap(ofGetElapsedTimef() - startedDying, 0, dyingTime, size, 0, true);
//    }
//    ofNoFill();
//    ofCircle(currentPos, size);
//    ofSetColor(color);
//    all.draw();
//    ofSetColor(255);
//    ofDrawBitmapString(ofToString(label), currentPos);
//    ofPopStyle();
    ofPushStyle();
    float size = 16;
    ofSetColor(255);
    if(startedDying){
        ofSetColor(ofColor::red);
        size = ofMap(ofGetElapsedTimef() - startedDying, 0, dyingTime, size, 0, true);
    }
    ofNoFill();
    ofSetLineWidth(2);
    ofSetColor(color);
    ofCircle(currentPos, size);
    ofSetColor(255);
    ofDrawBitmapString(ofToString(label), currentPos);
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
    }
    else if((currentTime - startedDying) > dyingTime){
        dead = true;
    }
}
