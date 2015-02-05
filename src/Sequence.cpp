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
        for(int i = 0; i < nMarkers; i++){
            if(i == 0) ofSetColor(255, 0, 0);
            if(i == 1) ofSetColor(0, 255, 0);
            frames[i].draw();

            ofPoint currentPoint = frames[i].getPointAtPercent(percent);
            pastPoints[i].addVertex(currentPoint);

            ofSetColor(0, 255, 0);
            pastPoints[i].draw();

            ofCircle(currentPoint, 3);
        }
    }
}

void Sequence::load(const string path){

	if(!ofFile::doesFileExist(path)) return;

    if(!xml.load(path)) return;

    // Clear polylines
    for(int i = 0; i < nMarkers; i++){
        frames[i].clear();
        pastPoints[i].clear();
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

    // Debug vertices of the sequence
//    for(int i = 0; i < nMarkers; i++){
//        vector<ofPoint> vertices = frames[i].getVertices();
//        for(int j = 0; j < vertices.size(); j++){
//           cout << j << ": " << vertices[j].x << " " << vertices[j].y << endl;
//        }
//    }

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
