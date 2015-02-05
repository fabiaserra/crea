#include "Sequence.h"

Sequence::Sequence(){
    recording = false;
    frame_counter = 0;
}

void Sequence::setup(int nMarkers){
    this->nMarkers = nMarkers;
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

    if(frames.size()){
        for(int i = 0; i < nMarkers; i++){
            ofSetColor(255, 0, 0);
            frames[i].draw();

//            float currentIndex = frames[i].getIndexAtPercent(percent);
//            ofPolyline completed;
//            for(vector<ofPoint>::iterator it = frames[i].begin(); it != frames[i].begin+currentIndex; it++){
//               completed.addVertex(*it);
//            }
//            completed.close();

//            ofPoint currentPoint = frames[i].getPointAtPercent(percent);
//            previousPoints[i].addVertex(currentPoint);
//            previousPoints[i].close();
//
//            ofSetColor(255, 0, 0);
//            previousPoints[i].draw();
        }
    }
}

void Sequence::load(const string path){

	if(!ofFile::doesFileExist(path)) return;

    if(!xml.load(path)) return;

    const size_t numFrames = xml.getNumTags("frame");
    float timestampFirstFrame;
    float timestampLastFrame;

    // Set up size of the sequence
    frames.resize(nMarkers);
    for(int i = 0; i < nMarkers; i++){
        frames[i].clear();
        frames[i].resize(numFrames);
    }

    // Load XML sequence in memory
    for(size_t frameIndex = 0; frameIndex < numFrames; frameIndex++){
        xml.pushTag("frame", frameIndex);

        // Frame timestamp
//        const float timestamp = xml.getValue("timestamp", -1.0);
        if(frameIndex == 0) timestampFirstFrame = xml.getValue("timestamp", -1.0);;
        if(frameIndex == numFrames-1) timestampLastFrame = xml.getValue("timestamp", -1.0);;

        // Frame markers positions
        const size_t nMarkers = xml.getNumTags("marker");
        for(size_t markerIndex = 0; markerIndex < nMarkers; markerIndex++){
            xml.pushTag("marker", markerIndex);
            const float px = xml.getValue("x", -1.0);
            const float py = xml.getValue("y", -1.0);
            frames[markerIndex].addVertex(ofPoint(px, py));
            xml.popTag();
        }
        xml.popTag();
    }

    // Close polylines
    for(int i = 0; i < nMarkers; i++){
        cout << frames[i].size() << endl;
        frames[i].close();
    }

    // Duration in seconds of the sequence
    duration = timestampLastFrame - timestampFirstFrame;
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
