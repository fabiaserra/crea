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
        frames.push_back(newPolyline);
    }

    // Set up previous points polylines
    for(int i = 0; i < nMarkers; i++){
        ofPolyline newPolyline;
        pastPoints.push_back(newPolyline);
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
            frames[markerIndex].draw();

            ofPoint currentPoint = frames[markerIndex].getPointAtPercent(percent);
            pastPoints[markerIndex].addVertex(currentPoint);

            ofSetColor(0, 255, 0);
            pastPoints[markerIndex].draw();

            ofCircle(currentPoint, 3);
        }
    }
}

void Sequence::load(const string path){

	if(!ofFile::doesFileExist(path)) return;

    if(!xml.load(path)) return;

    // Clear polylines
    for(int markerIndex = 0; markerIndex < nMarkers; markerIndex++){
        frames[markerIndex].clear();
        pastPoints[markerIndex].clear();
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
            const float px = xml.getValue("x", 86.0, markerIndex);
            const float py = xml.getValue("y", 86.0, markerIndex);
            frames[markerIndex].addVertex(ofPoint(px, py));
            xml.popTag();
        }
        xml.popTag();
    }

    // Debug: print vertices of the sequence
//    for(int i = 0; i < nMarkers; i++){
//        vector<ofPoint> vertices = frames[i].getVertices();
//        for(int j = 0; j < vertices.size(); j++){
//           cout << j << ": " << vertices[j].x << " " << vertices[j].y << endl;
//        }
//    }

    patterns.clear();

    // Break sequence in n fragments
    int nPatterns = 4;
    for(int patternIndex = 1; patternIndex <= nPatterns; patternIndex++){
        vector<ofPolyline> newPattern;
        for(int markerIndex = 0; markerIndex < frames.size(); markerIndex++){
            ofPolyline newPolyline;
            int cutIndex = frames[markerIndex].getIndexAtPercent(patternIndex * 1/nPatterns);
            for(int i = 0; i < cutIndex; i++){
                newPolyline.addVertex(frames[markerIndex].getVertices().at(i));
            }
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
