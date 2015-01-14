#include "Marker.h"

using namespace ofxCv;
using namespace cv;

const float dyingTime = 10;

Marker::Marker()
{
    startedDying = 0;
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
	if(startedDying == 0) {
		startedDying = currentTime;
	} else if(currentTime - startedDying > dyingTime) {
		dead = true;
	}
}
