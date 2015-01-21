#include "Marker.h"

using namespace ofxCv;
using namespace cv;

Marker::Marker()
{
    startedDying = 0;
    dyingTime = 10;
    timeDead = 0;
    bornRate = 3;
}

void Marker::setup(const cv::Rect& track) {
    color.setHsb(ofRandom(0, 255), 255, 255);
    currentPos = toOf(track).getCenter();
    smoothPos = currentPos;
    previousPos = currentPos;
}

void Marker::update(const cv::Rect& track) {
	currentPos = toOf(track).getCenter();
	smoothPos.interpolate(currentPos, .5);
	all.addVertex(smoothPos);
	velocity = smoothPos - previousPos;
	previousPos = smoothPos;
}

void Marker::update(vector<unsigned int> deadLabels, vector<unsigned int> currentLabels) {
    for(unsigned int i = 0; i < deadLabels.size(); i++) {
        if(deadLabels[i] == label) hasDisappeared = true;
    }

    for(unsigned int i = 0; i < currentLabels.size(); i++) {
        if(currentLabels[i] == label) hasDisappeared = false;
    }
}

void Marker::draw() {
	ofPushStyle();
	float size = 16;
	ofSetColor(255);
	if(startedDying) {
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

void Marker::kill() {
	float currentTime = ofGetElapsedTimef();
	timeDead = currentTime - startedDying;
	if(startedDying == 0) {
		startedDying = currentTime;
	} else if(timeDead > dyingTime) {
		dead = true;
	}
}
