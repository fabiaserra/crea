#pragma once

#include "ofMain.h"
#include "ofxUI.h"
#include "ofxCv.h"
#include "ofxKinect.h"

#include "ParticleSystem.h"
#include "hungarian.h"
#include "Marker.h"

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

//        vector< vector<int> > computeCostMatrix();

        void setupGUI0();
        void setupGUI1();
        void setupGUI2();
        void setupGUI3();

        void guiEvent(ofxUIEventArgs &e);
//--------------------------------------------------------------
        ofxKinect kinect;
//--------------------------------------------------------------
        bool flipKinect;
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
//        int nMarkers;                       //Number of IR markers to track
//        vector<Marker> newMarkers;          //Markers not yet identified
//        vector<Marker> markers;             //Markers identified with their unique ID
//        vector< vector<int> > costMatrix;   //Matrix of the cost (distance) to assign current IDs to each of the non identified markers
//--------------------------------------------------------------
//        ofxCv::KalmanPosition kalman;
//        ofVec2f point;
//        ofMesh predicted, line, estimated;
//        float speed;
//--------------------------------------------------------------
        map<int, ParticleSystem> particleSystems;
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
        //Particles parameters
        float bornRate;
        float velocity;
        float velocityRnd;
        float velocityMotion;
        float emitterSize;
        float lifetime;
        float lifetimeRnd;
        float radius;
        float radiusRnd;
        bool sizeAge;
        bool opacityAge;
        bool colorAge;
        bool bounce;
        float friction;
        float gravity;
        float turbulence;
//        EmitterType emitterType;
//--------------------------------------------------------------
        float smoothingSize;
};
