#pragma once

#include "ofMain.h"
#include "ofxUI.h"
#include "ofxCv.h"
#include "ofxKinect.h"

#include "ParticleSystem.h"
#include "Marker.h"

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
		void setupGUI6(int i);	// TODO: use the i to be able to create a vector of ParticleSystems

		void guiEvent(ofxUIEventArgs &e);

		//--------------------------------------------------------------
		ofxKinect kinect;
		//--------------------------------------------------------------
		bool  flipKinect;
		bool  resetKinect;
		float reScale;          // Ratio to scale the Image to full screen
		//--------------------------------------------------------------
		ofImage irImage, irOriginal;
		ofImage depthImage, depthOriginal;
		ofImage grayThreshNear;
		ofImage grayThreshFar;
		//--------------------------------------------------------------
		ofxCv::ContourFinder contourFinder;
		ofxCv::ContourFinder irMarkerFinder;
		ofxCv::RectTrackerFollower<Marker> tracker;
		//--------------------------------------------------------------
		float time0;            // Time value for computing dt
		//--------------------------------------------------------------
		// TODO: vector<ParticleSystem> particles
		ParticleSystem particles;
		ParticleSystem markersParticles;
		//--------------------------------------------------------------
		ofxUISuperCanvas *gui0;
		ofxUISuperCanvas *gui1;
		ofxUISuperCanvas *gui2;
		ofxUISuperCanvas *gui3;
		ofxUISuperCanvas *gui4;
		ofxUISuperCanvas *gui5;
		ofxUISuperCanvas *gui6;
		//--------------------------------------------------------------
        float dim;
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
		float smoothingSize;
		//--------------------------------------------------------------

        void guiEvent(ofxUIEventArgs &e);
//--------------------------------------------------------------
        ofxKinect kinect;
//--------------------------------------------------------------
        bool  flipKinect;
        float reScale;                      //Ratio to scale the Image to full screen
//--------------------------------------------------------------
        ofImage irImage, irOriginal;
        ofImage depthImage, depthOriginal;
        ofImage grayThreshNear;
        ofImage grayThreshFar;
//--------------------------------------------------------------
        ofxCv::ContourFinder contourFinder;
        ofxCv::ContourFinder irMarkerFinder;
        ofxCv::RectTrackerFollower<Marker> tracker;
//--------------------------------------------------------------
        float time0;                        //Time value for computing dt
//--------------------------------------------------------------
        ParticleSystem particles;
        ParticleSystem markersParticles;
//--------------------------------------------------------------
        ofPoint mousePos;
        ofPoint mousePrevPos;
        ofPoint mouseVel;
//--------------------------------------------------------------
        ofxUISuperCanvas *gui0;
        ofxUISuperCanvas *gui1;
        ofxUISuperCanvas *gui2;
        ofxUISuperCanvas *gui3;
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
        //DEPTH CONTOUR//
        float smoothingSize;

//------VMO Declaration-----------------------------------------
		vmo seqVmo;
		vmo::pttr pttrList;
		vmo::belief currentBf;
		vmo::belief prevBf;
	
		vector<vector<float> > obs;
		vector<vector<ofVec2f> > patterns;
		vector<vector<float> > loadXML(); // dummy function
		bool initStatus;
	
};
