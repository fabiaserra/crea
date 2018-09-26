/*
 * Copyright (C) 2015 Fabia Serra Arrizabalaga
 *
 * This file is part of Crea
 *
 * Crea is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the Free
 * Software Foundation (FSF), either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the Affero GNU General Public License
 * version 3 along with this program.  If not, see http://www.gnu.org/licenses/
 */

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
    ofDrawCircle(currentPos, size);
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
