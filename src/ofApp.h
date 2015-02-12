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

        void setupGUI0();
        void setupGUI1();
        void setupGUI2();
        void setupGUI3();
        void setupGUI4();
        void setupGUI5();
        void setupGUI6();
        void setupGUI7(int i);  // TODO: use the i to be able to create a vector of ParticleSystems

        void saveGUISettings(const string path);
        void loadGUISettings(const string path, bool triggerEvents);

        void loadCuesVector();
        void interpolateWidgetValues();

        void guiEvent(ofxUIEventArgs &e);

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
        //--------------------------------------------------------------
        vector<string> cues;
        int currentCueIndex;
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
        ofxUIImageToggle *recordingSequence;  // Button to record gestures sequence
        ofxUILabel *sequenceFilename;  // Button to record gestures sequence
        ofxUILabel *sequenceDuration;  // Button to record gestures sequence
        ofxUILabel *sequenceNumFrames;  // Button to record gestures sequence
        ofxUILabel *currentCueIndexLabel;  // Button to record gestures sequence
        ofxUITextInput *currentCueName;  // Button to record gestures sequence
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
