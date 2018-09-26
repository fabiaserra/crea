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

#include "Sequence.h"

Sequence::Sequence(){
    filename = "";
    duration = 0;
    numFrames = 0;
    playhead = 0;
    elapsedTime = 0;
    maxPatternsWindow = 10;
    verdana.load("fonts/verdana.ttf", 80, true, true);
}

void Sequence::setup(const int numMarkers){
    this->numMarkers = numMarkers;
}

void Sequence::update(){
    updatePlayhead();
}

void Sequence::record(const vector<irMarker>& markers){
    int frameNum = xml.addTag("frame");
    xml.pushTag("frame", frameNum);
    xml.setValue("timestamp", ofGetElapsedTimef(), frameNum);
    for(int markerIdx = 0; markerIdx < markers.size(); markerIdx++){
        int numMarker = xml.addTag("marker");
        xml.pushTag("marker", numMarker);
        xml.setValue("id", ofToString(markers[markerIdx].getLabel()), numMarker);
        xml.setValue("x",  markers[markerIdx].smoothPos.x, numMarker);
        xml.setValue("y",  markers[markerIdx].smoothPos.y, numMarker);
        xml.setValue("disappeared", markers[markerIdx].hasDisappeared, numMarker);
        xml.popTag();
    }
//    // fill with dummy markers if not enough markers
//    while(xml.getNumTags("marker") < numMarkers){
//        int numMarker = xml.addTag("marker");
//        xml.pushTag("marker", numMarker);
//        xml.setValue("id", -1, numMarker);
//        xml.setValue("x",  -1, numMarker);
//        xml.setValue("y",  -1, numMarker);
//        xml.setValue("disappeared", 1, numMarker);
//        xml.popTag();
//    }
    xml.popTag();
}

void Sequence::draw(){
    ofPushStyle();
    // Draw entire sequence
    for(int markerIdx = 0; markerIdx < numMarkers; markerIdx++){

        ofSetColor(255, 50);
        ofSetLineWidth(1.5);
        markersPosition[markerIdx].draw();

        ofPoint currentPoint;
        ofPolyline line;
        int currentIdx = calcCurrentFrameIndex();
        if(currentIdx >= 0){
            line.resize(currentIdx + 1);

            for(size_t idx = 0; idx <= currentIdx; idx++){
                currentPoint = markersPosition[markerIdx].getPointAtIndexInterpolated(idx);
                line[idx] = currentPoint;
            }

            ofColor c;
            if(markerIdx == 0) c.set(255, 0, 0);
            else if(markerIdx == 1) c.set(0, 0, 255);
            else if(markerIdx == 2) c.set(0, 255, 0);
            ofSetColor(c);
            ofSetLineWidth(2.5);
            line.draw();

            ofFill();
            c.setBrightness(150);
            ofSetColor(c);
            ofDrawCircle(currentPoint, 3);
        }
    }
    ofPopStyle();
}

void Sequence::load(const string path){

    if(!xml.load(path)) return;

    numMarkers = xml.getValue("numMarkers", numMarkers, 0);

    // Initialize polylines sequence
    markersPosition.clear();
    for(int i = 0; i < numMarkers; i++){
        ofPolyline newPolyline;
        markersPosition.push_back(newPolyline);
    }

    // Number of frames of the sequence
    numFrames = xml.getNumTags("frame");
    float timestampFirstFrame = -1;
    float timestampLastFrame = -1;
    size_t emptyFrames = 0;

    // Load XML sequence in memory
    for(size_t frameIdx = 0; frameIdx < numFrames; frameIdx++){
        xml.pushTag("frame", frameIdx);

        const size_t frameNumMarkers = xml.getNumTags("marker");

        // Frame timestamp
        if(timestampFirstFrame == -1 && frameNumMarkers >= numMarkers) timestampFirstFrame = xml.getValue("timestamp", -2.0);
        if(frameNumMarkers >= numMarkers) timestampLastFrame = xml.getValue("timestamp", -2.0);

        // If no recorded markers or less than numMarkers we don't consider that frame
        if(frameNumMarkers < numMarkers){
            emptyFrames++;
            xml.popTag();
            continue;
        }

        int addedMarkers = 0;
        for(size_t markerIdx = 0; markerIdx < frameNumMarkers; markerIdx++){
            xml.pushTag("marker", markerIdx);
            unsigned int id = xml.getValue("id", -2.0);
            const float px = xml.getValue("x", -2.0);
            const float py = xml.getValue("y", -2.0);
            const bool disappeared = xml.getValue("disappeared", -1.0);

            // Number of markers we still have to take the value from
            int aheadMarkers = (frameNumMarkers-1) - markerIdx;
            // If marker is not disappeared in that frame we add it
            if(!disappeared){
                markersPosition[addedMarkers].addVertex(ofPoint(px, py));
                addedMarkers++;
            }
            // If marker is disappeared but not enough markers to fill numMarkers we add it too
            else if(disappeared && aheadMarkers < (numMarkers-addedMarkers)){
                markersPosition[addedMarkers].addVertex(ofPoint(px, py));
                addedMarkers++;
            }
            // If marker has disappeared but we have other markers in the list we don't add it
            else{}

            xml.popTag();

            // If we have already loaded more or equal numMarkers we go to the next frame
            if(addedMarkers >= numMarkers ){
                break;
            }
        }

//        // If not enough markers in the frame to fill numMarkers we add dummy vertices to the polyline
//        while(addedMarkers < numMarkers){
//            markersPosition[addedMarkers].addVertex(ofPoint(-1, -1));
//            addedMarkers++;
//        }

        xml.popTag();
    }

    filename = ofFilePath::getBaseName(path);

//    // Create n equal length patterns for debug
//    createPatterns(6);

    // Number of frames of the sequence
    numFrames = numFrames - emptyFrames;

    // Duration in seconds of the sequence
    duration = timestampLastFrame - timestampFirstFrame;
}

// Draw patterns inside the long sequence
void Sequence::drawPatterns(map<int, float>& currentPatterns){
    ofPushStyle();
    // Draw gesture patterns
    int nPatterns = patterns.size();
    for(int patternIdx = 0; patternIdx < nPatterns; patternIdx++){
        bool highlight = false;
        float percent = 0;
        // If the pattern is is inside the map
        if(currentPatterns.find(patternIdx) != currentPatterns.end()){
            highlight = true;
            percent = currentPatterns[patternIdx];
        }

        ofColor c = ofColor::fromHsb(0, 255, 255);
        c.setHue(ofMap(patternIdx, 0, nPatterns-1, 0, 255));

        for(int markerIdx = 0; markerIdx < numMarkers; markerIdx++){
            int opacity = 100;
            if(highlight) opacity = 170;

            c.setBrightness(255);
            ofSetColor(c, opacity);
            ofSetLineWidth(2.5);
            patterns[patternIdx][markerIdx].draw();

            if(highlight){
                if(percent > 0.99) percent = 0.99;
                ofPoint currentPoint;
                ofPolyline line;
                for(float p = 0.0; p <= percent; p += 0.001){
                    currentPoint = patterns[patternIdx][markerIdx].getPointAtPercent(p);
                    line.addVertex(currentPoint);
                }

                ofSetColor(c, 255);
                ofSetLineWidth(2);
                line.draw();

                // Pattern current processing point
                c.setBrightness(150);
                ofSetColor(c, 255);
                ofDrawCircle(currentPoint, 3);
            }
        }
    }
    ofPopStyle();
}

// Draw patterns separate in a side of the screen
void Sequence::drawPatternsSeparate(map<int, float>& currentPatterns){
    // If there are more patterns than maximum able to show in the window
    int nPatterns = MIN(maxPatternsWindow, patterns.size());

    // TODO: show most important patterns if there are more than maxPatternsToShow

    // Draw gesture patterns
    for(int patternIdx = 0; patternIdx < nPatterns; patternIdx++){
        int patternPosition = patternIdx + 1;
        bool highlight = false;
        float percent = 0;
        // If the pattern is is inside the map
        if(currentPatterns.find(patternIdx) != currentPatterns.end()){
            highlight = true;
            percent = currentPatterns[patternIdx];
        }
        drawPattern(patternPosition, patternIdx, percent, highlight);
    }
}

void Sequence::drawPattern(const int patternPosition, const int patternIdx, float percent, const bool highlight){

    // Drawing window parameters
    float width = 640.0;
    float height = 480.0;
    float scale = 5.5;
    float margin = 40.0;
    float guiWidth = 1330;
    float guiHeight = 1430;

    ofPushMatrix();
    ofPushStyle();

    ofScale(1.0/scale, 1.0/scale);
    ofTranslate(guiWidth, guiHeight);

    // Position window pattern in the screen
    if(patternPosition%2 == 0){
        ofTranslate(guiWidth+width+margin, (patternPosition/2 - 1) * (height+margin));
    }
    else{
        ofTranslate(guiWidth, ((patternPosition+1)/2 - 1) * (height+margin));
    }

    int opacity = 60;
    if(highlight) opacity = 255;

    // Background pattern window box
    ofSetColor(0);
    ofFill();
    ofDrawRectangle(0, 0, width, height);

    // Contour pattern window box
    ofPushStyle();
    ofSetColor(255, opacity);
    ofSetLineWidth(2);
    ofNoFill();
    ofDrawRectangle(0, 0, width, height);
    ofPopStyle();

    for(int markerIdx = 0; markerIdx < numMarkers; markerIdx++){
        // Pattern lines
        ofSetColor(120, opacity);
        ofSetLineWidth(2);
        patterns[patternIdx][markerIdx].draw();

        if(highlight){
            if(percent > 0.99) percent = 0.99; // so it doesn't jump to the beginning

            ofPoint currentPoint;
            ofPolyline line;
            for(float p = 0.0; p <= percent; p += 0.001){
                currentPoint = patterns[patternIdx][markerIdx].getPointAtPercent(p);
                line.addVertex(currentPoint);
            }

            // Pattern already processed points
            ofSetColor(255);
            ofSetLineWidth(2);
            line.draw();

            // Pattern current processing point
            ofSetColor(240, 0, 20, opacity);
            ofDrawCircle(currentPoint, 10);
        }
    }

    // Pattern label number
    ofSetColor(255, opacity+30);
    verdana.drawString(ofToString(patternIdx+1), 30, 100);

    ofPopStyle();
    ofPopMatrix();
}

// Draw current point in the tracking of the sequence
void Sequence::drawTracking(int currentIdx){
    if(currentIdx >= 0){
        ofPushStyle();
        for(int markerIdx = 0; markerIdx < numMarkers; markerIdx++){
            ofColor c(0,255,0);

//            // Draw all past points
//            ofPolyline line;
//            line.resize(currentIdx + 1);
//
//            for(size_t idx = 0; idx <= currentIdx; idx++){
//                ofPoint point = markersPosition[markerIdx].getPointAtIndexInterpolated(idx);
//                line[idx] = point;
//            }
//
//            ofSetColor(c);
//            ofSetLineWidth(2.5);
//            line.draw();

            // Current point
            ofPoint currentPoint;
            currentPoint = markersPosition[markerIdx][currentIdx];

            ofFill();
            ofSetColor(c);
            ofDrawCircle(currentPoint, 5);

        }
        ofPopStyle();
    }
}

// Draw the segment of the sequence that belongs to the different cues
void Sequence::drawSegments(){
    ofPushStyle();
    for(int segmentIdx = 0; segmentIdx < segments.size(); segmentIdx++){

        ofColor c = ofColor::fromHsb(0, 255, 255);
        c.setHue(ofMap(segmentIdx, 0, segments.size(), 0, 255));

//        ofColor c(255);
//        if(segmentIdx == 0) c.set(0, 0, 255);
//        else if(segmentIdx == 1) c.set(0, 255, 0);

        ofSetLineWidth(4);
        ofSetColor(c);
        for(int markerIdx = 0; markerIdx < numMarkers; markerIdx++){
            segments[segmentIdx][markerIdx].draw();
        }
    }
    ofPopStyle();
}

// Update the segments of the sequence that belong to the different cues
void Sequence::updateSegments(const vector< pair<float, float> >& segmentsPcts){
    // Clear sequence segments polylines
    for(int segmentIdx = 0; segmentIdx < segments.size(); segmentIdx++){
        for(int markerIdx = 0; markerIdx < numMarkers; markerIdx++){
            segments[segmentIdx][markerIdx].clear();
        }
        segments[segmentIdx].clear();
    }
    segments.clear();

    for(int segmentIdx = 0; segmentIdx < segmentsPcts.size(); segmentIdx++){
        vector<ofPolyline> segment = getSegment(segmentsPcts[segmentIdx]);
        segments.push_back(segment);
    }
}

vector<ofPolyline> Sequence::getSegment(const pair<float, float>& segmentPctRange){
    float lowPct = segmentPctRange.first/100.0;
    float highPct = segmentPctRange.second/100.0;

    vector<ofPolyline> segment;
    float increment = 0.0001; // TODO: change it depending on how many frames has the sequence?
    for(int markerIdx = 0; markerIdx < numMarkers; markerIdx++){
        ofPolyline markerSegment;
//        float increment = ofMap(markersPosition[markerIdx].getPerimeter(), 0, 15000, 0.005, 0.0001, true);
        for(float pct = lowPct; pct < highPct; pct += increment){
            ofPoint p = markersPosition[markerIdx].getPointAtPercent(pct);
            markerSegment.addVertex(p);
        }
        segment.push_back(markerSegment);
    }
    return segment;
}

void Sequence::loadPatterns(vector< vector<ofPolyline> > patterns){
    this->patterns = patterns;
}

// Create some dummy patterns from the long sequence
void Sequence::createPatterns(int nPatterns){
    // Clear and initialize memory of polylines patterns
    patterns.clear();
    for(int patternIdx = 0; patternIdx < nPatterns; patternIdx++){
        vector<ofPolyline> newPattern;
        for(int markerIdx = 0; markerIdx < numMarkers; markerIdx++){
            ofPolyline newPolyline;
            newPattern.push_back(newPolyline);
        }
        patterns.push_back(newPattern);
    }

    // Break sequence in n patterns
    for(int patternIdx = 0; patternIdx < nPatterns; patternIdx++){
        for(int markerIdx = 0; markerIdx < numMarkers; markerIdx++){
            int startIdx = markersPosition[markerIdx].getIndexAtPercent(patternIdx * (1.01/nPatterns));
            int endIdx = markersPosition[markerIdx].getIndexAtPercent((patternIdx+1) * (1.01/nPatterns))+1;
            if (endIdx == 1) endIdx = markersPosition[markerIdx].size();
            for(int i = startIdx; i < endIdx; i++){
                patterns[patternIdx][markerIdx].addVertex(markersPosition[markerIdx][i]);
            }
        }
    }
}

void Sequence::save(const string path) {
    xml.saveFile(path);
}

void Sequence::startRecording(){
    xml.clear();
    xml.setValue("numMarkers", numMarkers, 0);
}

void Sequence::clearPlayback(){
    playhead = 0;
    elapsedTime = 0;
}

void Sequence::updatePlayhead()
{
    elapsedTime += ofGetLastFrameTime();

    if(elapsedTime > duration){
        clearPlayback();
    }

    playhead = (elapsedTime / duration);
}

size_t Sequence::calcCurrentFrameIndex()
{
    size_t frameIdx = floor(numFrames * playhead);

    if(frameIdx >= numFrames) frameIdx = numFrames-1;

    return frameIdx;
}

float Sequence::getCurrentPercent(int currentIdx){
    return markersPosition.at(0).getLengthAtIndexInterpolated(currentIdx) / markersPosition.at(0).getPerimeter();
}

ofPoint Sequence::getCurrentPoint(int markerIdx){
    return markersPosition[markerIdx].getPointAtIndexInterpolated(calcCurrentFrameIndex());
}

int Sequence::getNumMarkers(){
    return numMarkers;
}
