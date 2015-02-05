#include "Sequence.h"

Sequence::Sequence(){
    recording = false;
    sequenceLoaded = false;
    frame_counter = 0;
}

void Sequence::setup(int nMarkers){
    this->nMarkers = nMarkers;

    // Set up sequence polylines
    for(int i = 0; i < nMarkers; i++){
        ofPolyline newPolyline;
        markersPosition.push_back(newPolyline);
    }

    // Set up previous points polylines
    for(int i = 0; i < nMarkers; i++){
        ofPolyline newPolyline;
        markersPastPoints.push_back(newPolyline);
    }

}

void Sequence::update(vector<Marker>& markers){
    if(recording){
        int frameNum = xml.addTag("frame");
        xml.pushTag("frame", frameNum);
        xml.setValue("timestamp", ofGetElapsedTimef(), frameNum);
        for(int i = 0; i < nMarkers; i++){
            int numMarker = xml.addTag("marker");
            xml.pushTag("marker", numMarker);
//            xml.setValue("id", i, numMarker);
            xml.setValue("x", markers[i].smoothPos.x, numMarker);
            xml.setValue("y", markers[i].smoothPos.y, numMarker);
            xml.popTag();
        }
        xml.popTag();
    }
}

void Sequence::draw(float percent){
    if(sequenceLoaded){
        for(int markerIndex = 0; markerIndex < nMarkers; markerIndex++){
            if(markerIndex == 0) ofSetColor(255, 0, 0);
            if(markerIndex == 1) ofSetColor(0, 255, 0);
            markersPosition[markerIndex].draw();

//            ofPoint currentPoint = markersPosition[markerIndex].getPointAtPercent(percent);
//            markersPastPoints[markerIndex].addVertex(currentPoint);
//
//            ofSetColor(0, 255, 0);
//            markersPastPoints[markerIndex].draw();
//
//            ofCircle(currentPoint, 3);
        }

        for(int patternIndex = 0; patternIndex < patterns.size(); patternIndex++){
            if(patternIndex%2 == 0) ofSetColor(0, 255, 255);
            else ofSetColor(0, 0, 255);
            for(int markerIndex = 0; markerIndex < patterns[patternIndex].size(); markerIndex++){
                patterns[patternIndex][markerIndex].draw();
//                cout << "hello";
            }
        }
    }
}

void Sequence::load(const string path){

	if(!ofFile::doesFileExist(path)) return;

    if(!xml.load(path)) return;

    // Clear polylines
    for(int markerIndex = 0; markerIndex < nMarkers; markerIndex++){
        markersPosition[markerIndex].clear();
        markersPastPoints[markerIndex].clear();
    }

    // Load XML sequence in memory

    // Number of frames of the sequence
    const size_t numFrames = xml.getNumTags("frame");
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
            const float px = xml.getValue("x", -1.0, markerIndex);
            const float py = xml.getValue("y", -1.0, markerIndex);
            markersPosition[markerIndex].addVertex(ofPoint(px, py));
            xml.popTag();
        }
        xml.popTag();
    }

    // Debug: print vertices of the sequence
//    for(int i = 0; i < nMarkers; i++){
//        vector<ofPoint> vertices = markersPosition[i].getVertices();
//        for(int j = 0; j < vertices.size(); j++){
//           cout << j << ": " << vertices[j].x << " " << vertices[j].y << endl;
//        }
//    }

    patterns.clear();

    // Break sequence in n fragments
    int nPatterns = 4;

    for(int patternIndex = 1; patternIndex <= nPatterns; patternIndex++){
        vector<ofPolyline> newPattern;
        for(int markerIndex = 0; markerIndex < markersPosition.size(); markerIndex++){
            ofPolyline newPolyline;
            int startIndex = markersPosition[markerIndex].getIndexAtPercent((patternIndex-1) * (1.01/nPatterns));
            int endIndex = markersPosition[markerIndex].getIndexAtPercent(patternIndex * (1.01/nPatterns))+1;
            if (endIndex == 0) endIndex = markersPosition[markerIndex].size();
            for(int i = startIndex; i < endIndex; i++){
                cout << i << " ";
                newPolyline.addVertex(markersPosition[markerIndex][i]);
            }
            cout << endl;
            newPattern.push_back(newPolyline);
        }
        patterns.push_back(newPattern);
    }

    cout << patterns.size() << endl;

    // Duration in seconds of the sequence
    duration = timestampLastFrame - timestampFirstFrame;

    sequenceLoaded = true;
}


void Sequence::save(const string path) {
    xml.saveFile(path);
}

void Sequence::startRecording(){
	xml.clear();
	recording = true;
}

void Sequence::stopRecording(){
	recording = false;
}
