#include "Sequence.h"

Sequence::Sequence(){
    recording = false;
    frame_counter = 0;
}

void Sequence::setup(){

}

void Sequence::update(vector<Marker>& markers){
	int numMarkers = 2;

    if(recording){
        int frameNum = xml.addTag("frame");
        xml.pushTag("frame", frameNum);
        xml.setValue("timestamp", ofGetElapsedTimef(), frameNum);
        for(int i = 0; i < numMarkers; i++){
            int numMarker = xml.addTag("marker");
            xml.pushTag("marker", numMarker);
            xml.setValue("id", i, numMarker);
            xml.setValue("x", ofGetAppPtr()->mouseX, numMarker);
            xml.setValue("y", ofGetAppPtr()->mouseY, numMarker);
            xml.popTag();
        }
        xml.popTag();
    }
}

void Sequence::load(const string path){

	if(!ofFile::doesFileExist(path)) return;

    xml.load(path);

    const size_t num_frames = xml.getNumTags("frame");

    sequence.clear();
    for(size_t frame_index = 0; frame_index < num_frames; frame_index++){
        xml.pushTag("frame", frame_index);
        const float timestamp = xml.getValue("timestamp", -1.0);
        const size_t num_markers = xml.getNumTags("marker");
        vector<ofPoint> markers;
        for(size_t marker_index = 0; marker_index < num_markers; marker_index++){
            xml.pushTag("marker", marker_index);
            const float px = xml.getValue("x", -1.0);
            const float py = xml.getValue("y", -1.0);
            markers.push_back(ofPoint(px, py));
            xml.popTag();
        }
        xml.popTag();
        sequence.push_back(markers);
    }

    line.clear();
    for(int i = 0; i < sequence.size(); i++){
        for(int j = 0; j < sequence[i].size(); j++){
            ofPoint point = sequence[i][j];
            line.addVertex(point);
        }
    }
    line.close();
}

void Sequence::startRecording(){
	recording = true;
}
