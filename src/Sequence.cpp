#include "Sequence.h"

Sequence::Sequence(){
    filename = "";
    duration = 0;
    numFrames = 0;
    playhead = 0;
    elapsed_time = 0;
}

void Sequence::setup(int nMarkers){
    this->nMarkers = nMarkers;

    verdana.loadFont("fonts/verdana.ttf", 80, true, true);

}

void Sequence::update(){
    updatePlayhead();
}

void Sequence::record(vector<irMarker>& markers){
    int frameNum = xml.addTag("frame");
    xml.pushTag("frame", frameNum);
    xml.setValue("timestamp", ofGetElapsedTimef(), frameNum);
    for(int i = 0; i < nMarkers; i++){
        int numMarker = xml.addTag("marker");
        xml.pushTag("marker", numMarker);
//        xml.setValue("id", i, numMarker);
        xml.setValue("x", markers[i].smoothPos.x, numMarker);
        xml.setValue("y", markers[i].smoothPos.y, numMarker);
        xml.popTag();
    }
    xml.popTag();
}

void Sequence::draw(){

    // Draw entire sequence
    for(int markerIndex = 0; markerIndex < nMarkers; markerIndex++){
        if(markerIndex == 0) ofSetColor(255, 0, 0);
        if(markerIndex == 1) ofSetColor(0, 255, 0);
        markersPosition[markerIndex].draw();
        
        ofPoint currentPoint = markersPosition[markerIndex].getPointAtIndexInterpolated(calcCurrentFrameIndex());
        markersPastPoints[markerIndex].addVertex(currentPoint);

        ofSetColor(0, 255, 0);
        markersPastPoints[markerIndex].draw();

        ofCircle(currentPoint, 3);
    }
}

void Sequence::load(const string path){

    if(!ofFile::doesFileExist(path)) return;

    if(!xml.load(path)) return;

    // Initialize polylines sequence
    markersPosition.clear();
    for(int i = 0; i < nMarkers; i++){
        ofPolyline newPolyline;
        markersPosition.push_back(newPolyline);
    }

    // Initialize previous points polylines sequence
    markersPastPoints.clear();
    for(int i = 0; i < nMarkers; i++){
        ofPolyline newPolyline;
        markersPastPoints.push_back(newPolyline);
    }

    // Load XML sequence in memory

    // Number of frames of the sequence
    numFrames = xml.getNumTags("frame");
    float timestampFirstFrame;
    float timestampLastFrame;

    for(size_t frameIndex = 0; frameIndex < numFrames; frameIndex++){
        xml.pushTag("frame", frameIndex);

        // Frame timestamp
        if(frameIndex == 0) timestampFirstFrame = xml.getValue("timestamp", -1.0);;
        if(frameIndex == numFrames-1) timestampLastFrame = xml.getValue("timestamp", -1.0);;

        // Frame markers positions
        const size_t nMarkers = xml.getNumTags("marker");
        for(size_t markerIndex = 0; markerIndex < nMarkers; markerIndex++){
            xml.pushTag("marker", markerIndex);
            const float px = xml.getValue("x", -1.0);
            const float py = xml.getValue("y", -1.0);
            markersPosition[markerIndex].addVertex(ofPoint(px, py));
            xml.popTag();
        }
        xml.popTag();
    }

    filename = ofFilePath::getFileName(path);

    // // Debug: print vertices of the sequence
    // for(int i = 0; i < nMarkers; i++){
    //     vector<ofPoint> vertices = markersPosition[i].getVertices();
    //     for(int j = 0; j < vertices.size(); j++){
    //        cout << j << ": " << vertices[j].x << " " << vertices[j].y << endl;
    //     }
    // }
//
//    int nPatterns = 14;
//
//    // Clear and initialize memory of polylines patterns
//    patterns.clear();
//    for(int patternIndex = 0; patternIndex < nPatterns; patternIndex++){
//        vector<ofPolyline> newPattern;
//        for(int markerIndex = 0; markerIndex < nMarkers; markerIndex++){
//            ofPolyline newPolyline;
//            newPattern.push_back(newPolyline);
//        }
//        patterns.push_back(newPattern);
//    }
//
//    // Clear and initialize memory of previous points polylines patterns
//    patternsPastPoints.clear();
//    for(int patternIndex = 0; patternIndex < nPatterns; patternIndex++){
//        vector<ofPolyline> newPattern;
//        for(int markerIndex = 0; markerIndex < nMarkers; markerIndex++){
//            ofPolyline newPolyline;
//            newPattern.push_back(newPolyline);
//        }
//        patternsPastPoints.push_back(newPattern);
//    }

//    // Break sequence in n patterns for debug
//    for(int patternIndex = 0; patternIndex < nPatterns; patternIndex++){
//        for(int markerIndex = 0; markerIndex < nMarkers; markerIndex++){
//            int startIndex = markersPosition[markerIndex].getIndexAtPercent(patternIndex * (1.01/nPatterns));
//            int endIndex = markersPosition[markerIndex].getIndexAtPercent((patternIndex+1) * (1.01/nPatterns))+1;
//            if (endIndex == 1) endIndex = markersPosition[markerIndex].size();
//            for(int i = startIndex; i < endIndex; i++){
//                patterns[patternIndex][markerIndex].addVertex(markersPosition[markerIndex][i]);
//            }
//        }
//    }

    // Duration in seconds of the sequence
    duration = timestampLastFrame - timestampFirstFrame;
}

void Sequence::drawPatterns(map<int, float> currentPatterns){
    // Draw gesture patterns
    for(int patternIndex = 0; patternIndex < patterns.size(); patternIndex++){
        int patternPosition = patternIndex + 1;
        bool highlight = false;
        float percent = 0;
        // if the pattern is is inside the map
        if(currentPatterns.find(patternIndex) != currentPatterns.end()) {
            highlight = true;
            percent = currentPatterns[patternIndex];
        }
        drawPattern(patternPosition, patternIndex, percent, highlight);
    }
}

void Sequence::drawPattern(int patternPosition, int patternIndex, float percent, bool highlight){

    // Drawing window parameters
    float width = 640.0;
    float height = 480.0;
    float scale = 5.5;
    float margin = 40.0;
    float guiHeight = 800;

    ofPushMatrix();

        ofScale(1.0/scale, 1.0/scale);
        ofTranslate(0, guiHeight);

        // Position window pattern
        if(patternPosition%2 == 0){
            ofTranslate(width+margin, (patternPosition/2 - 1) * (height+margin));
        }
        else{
            ofTranslate(0, ((patternPosition+1)/2 - 1) * (height+margin));
        }

        int opacity;
        if(highlight) opacity = 255;
        else          opacity = 60;

        // Background pattern window box
        ofSetColor(0);
        ofFill();
        ofRect(0, 0, width, height);

        // Contour pattern window box
        ofSetColor(255, opacity);
        ofSetLineWidth(2);
        ofNoFill();
        ofRect(0, 0, width, height);

        for(int markerIndex = 0; markerIndex < patterns[patternIndex].size(); markerIndex++){
            // Pattern lines
            ofSetColor(120, opacity);
            ofSetLineWidth(2);
            patterns[patternIndex][markerIndex].draw();

            ofPoint currentPoint = patterns[patternIndex][markerIndex].getPointAtPercent(percent);
            patternsPastPoints[patternIndex][markerIndex].addVertex(currentPoint);

            // Pattern already processed lines
            ofSetColor(255, opacity);
            ofSetLineWidth(3);
            patternsPastPoints[patternIndex][markerIndex].draw();

            // Pattern current processing point
            ofSetColor(240, 0, 20, opacity);
            ofCircle(currentPoint, 10);
        }

        // Pattern label number
        ofSetColor(255, opacity+30);
        verdana.drawString(ofToString(patternIndex+1), 30, 100);

    ofPopMatrix();
}

//void Sequence::setPatterns(vector< vector<ofPolyline> > patterns){
//    this->patterns = patterns;
//
//    // Clear and initialize memory of previous points polylines patterns
//    patternsPastPoints.clear();
//    for(int patternIndex = 0; patternIndex < patterns.size(); patternIndex++){
//        vector<ofPolyline> newPattern;
//        for(int markerIndex = 0; markerIndex < nMarkers; markerIndex++){
//            ofPolyline newPolyline;
//            newPattern.push_back(newPolyline);
//        }
//        patternsPastPoints.push_back(newPattern);
//    }
//}

void Sequence::save(const string path) {
    xml.saveFile(path);
}

void Sequence::startRecording(){
    xml.clear();
}

void Sequence::updatePlayhead()
{
    elapsed_time += ofGetLastFrameTime();
    
    if (elapsed_time > duration)
    {
        elapsed_time = 0;
    }
    
    if (elapsed_time < 0)
    {
        elapsed_time = duration;
    }
    
    playhead = (elapsed_time / duration);
    elapsed_time = duration * playhead;
}

size_t Sequence::calcCurrentFrameIndex()
{
    size_t frameIndex = floor(numFrames * playhead);
    
    if (frameIndex >= numFrames) frameIndex = numFrames-1;
    
    return frameIndex;
}

