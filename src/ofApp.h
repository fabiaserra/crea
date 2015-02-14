#pragma once

#include "ofMain.h"
#include "ofxUI.h"
#include "ofxCv.h"
#include "ofxKinect.h"

#include "ParticleSystem.h"
#include "irMarker.h"
#include "Contour.h"

#include "Sequence.h"

// Include VMO files here
//-----------------------
#include "vmo.h"
//-----------------------

class ofApp : public ofBaseApp{

    public:
        void setup();
        void update();
        void draw();

        void setupGUI0();
        void setupGUI1();
        void setupGUI2();
        void setupGUI3();
        void setupGUI4();
        void setupGUI5();
        void setupGUI6();
        void setupGUI7(int i);  // TODO: use the i to be able to create a vector of ParticleSystems

        void saveGUISettings(const string path, const bool saveCues);
        void loadGUISettings(const string path, const bool interpolate, const bool loadCues);
        void interpolateWidgetValues();

        void guiEvent(ofxUIEventArgs &e);

        void exit();

        void keyPressed(int key);
        void keyReleased(int key);
        void mouseMoved(int x, int y );
        void mouseDragged(int x, int y, int button);
        void mousePressed(int x, int y, int button);
        void mouseReleased(int x, int y, int button);
        void windowResized(int w, int h);
        void dragEvent(ofDragInfo dragInfo);
        void gotMessage(ofMessage msg);

        //--------------------------------------------------------------
        float time0;            // Time value for computing dt
        //--------------------------------------------------------------
        ofxKinect kinect;
        //--------------------------------------------------------------
        bool  flipKinect;       // Flip kinect image
        bool  resetKinect;      // Reset kinect
        float reScale;          // Ratio to scale the Image to full screen
        //--------------------------------------------------------------
        ofImage irImage, irOriginal;
        ofImage depthImage, depthOriginal;
        ofImage grayThreshNear;
        ofImage grayThreshFar;
        //--------------------------------------------------------------
        ofxCv::ContourFinder contourFinder;
        ofxCv::ContourFinder irMarkerFinder;
        ofxCv::RectTrackerFollower<irMarker> tracker;
        //--------------------------------------------------------------
        vector<irMarker> markers;
        //--------------------------------------------------------------
        // TODO: vector<ParticleSystem> particles
        ParticleSystem particles;
        ParticleSystem markersParticles;
        //--------------------------------------------------------------
        Contour contour;        // User silhouette contour
        //--------------------------------------------------------------
        Sequence sequence;      // Gestures sequence
        bool drawPatterns;
        bool drawSequence;
        float testCounter;      // delete this when VMO tracking done
        //--------------------------------------------------------------
        vector<string> cues;
        int currentCueIndex;
        bool interpolatingWidgets;
        map<ofxUIWidget *, vector<float> > widgetsToUpdate;
        //--------------------------------------------------------------
        vector<ofxUISuperCanvas *> guis;
        ofxUISuperCanvas *gui0;
        ofxUISuperCanvas *gui1;
        ofxUISuperCanvas *gui2;
        ofxUISuperCanvas *gui3;
        ofxUISuperCanvas *gui4;
        ofxUISuperCanvas *gui5;
        ofxUISuperCanvas *gui6;
        ofxUISuperCanvas *gui7;
        //--------------------------------------------------------------
        ofxUIImageToggle *recordingSequence;  // Toggle to record gestures sequence
        ofxUILabel *sequenceFilename;         // Name of the sequence
        ofxUILabel *sequenceDuration;         // Duration of the sequence in seconds
        ofxUILabel *sequenceNumFrames;        // Number of frames of the sequence
        ofxUILabel *cueIndexLabel;            // Current cue index
        ofxUITextInput *cueName;              // Name of the cue
        int theme;
        //--------------------------------------------------------------
        float dim;              // Size of GUI elements
        float guiWidth;
        //--------------------------------------------------------------
        float red, green, blue;
        //--------------------------------------------------------------
        float nearClipping, farClipping;
        float nearThreshold, farThreshold;
        float minContourSize, maxContourSize;
        //--------------------------------------------------------------
        float irThreshold;
        float minMarkerSize, maxMarkerSize;
        float trackerPersistence;
        float trackerMaxDistance;
        //--------------------------------------------------------------
        // float smoothingSize;
        //--------------------------------------------------------------

        //------VMO Declaration-----------------------------------------
        vmo seqVmo;
        vmo::pttr pttrList;
        vmo::belief currentBf;
        vmo::belief prevBf;

        // vector<ofPoint>& loadedFrames;
        vector<vector<float> > obs;
        // vector<vector<ofPolyline> > patterns;
        vector<vector<float> > loadXML(); // dummy function
        bool initStatus;
        bool stopTracking;
        map<int, float> gestureUpdate;

};
