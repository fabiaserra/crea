#pragma once

// Addons
//-----------------------
#include "ofMain.h"
#include "ofxUI.h"
#include "ofxCv.h"
#include "ofxKinect.h"
#include "ofxSecondWindow.h"

// Classes
//-----------------------
#include "ParticleSystem.h"
#include "irMarker.h"
#include "Contour.h"
#include "Sequence.h"

// VMO files
//-----------------------
#include "vmo.h"
#include "helper.h"

// comment this to use the recorded images
//#define KINECT_CONNECTED

// Uncomment this to use an xml sequence file for the tracking
//#define KINECT_SEQUENCE

class ofApp : public ofBaseApp{

    public:
        void setup();
        void update();
        void draw();
    
        void setupHelperGUI();
        void setupBasicsGUI();
        void setupKinectGUI();
        void setupGesturesGUI();
        void setupCueListGUI();
        void setupFluidSolverGUI();
        void setupContourGUI();
        void setupEmitterGUI();
        void setupGridGUI();
        void setupBoidsGUI();
        void setupAnimationsGUI();

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

        void addParticleBasicsGUI(ofxUICanvas* gui, ParticleSystem* ps);
        void addParticleInteractionGUI(ofxUICanvas* gui, ParticleSystem* ps);
        void addParticlePropertiesGUI(ofxUICanvas* gui, ParticleSystem* ps);
        void addParticlePhysicsGUI(ofxUICanvas* gui, ParticleSystem* ps);

        //--------------------------------------------------------------
        float time0;            // Time value for computing dt
        //--------------------------------------------------------------
        ofxKinect kinect;
        //--------------------------------------------------------------
        vector<ofImage *> savedDepthImages; // the saved Depth images for playback
        vector<ofImage *> savedIrImages;    // the saved IR images for playback
        //--------------------------------------------------------------
        Sequence kinectSequence;            // offline sequence for vmo tracking
        //--------------------------------------------------------------
        bool  flipKinect;       // Flip kinect image
        bool  resetKinect;      // Reset kinect
        int   angle;            // Tilt angle of the kinect
//        ofVec2f reScale;        // Ratios to scale the Image to full screen
        float reScale;          // Ratio to scale the Image to full screen
        int numDilates;         // Number of dilates applied to depth image
        int numErodes;          // Number of erodes applied to depth image
        int blurValue;          // Size of the blur filter
        //--------------------------------------------------------------
        ofImage irImage, irOriginal;
        ofImage depthImage, depthOriginal;
        ofImage grayThreshNear;
        ofImage grayThreshFar;
        //--------------------------------------------------------------
        cv::Mat croppingMask;
        float leftMask;
        float rightMask;
        float topMask;
        float bottomMask;
        //--------------------------------------------------------------
        ofxCv::ContourFinder irMarkerFinder;
        ofxCv::RectTrackerFollower<irMarker> tracker;
        //--------------------------------------------------------------
//        vector<irMarker> markers;
        int numMarkers;
        //--------------------------------------------------------------
        ParticleSystem *emitterParticles;
        ParticleSystem *gridParticles;
        ParticleSystem *boidsParticles;
        ParticleSystem *animationsParticles;
        vector<ParticleSystem *> particleSystems;
        int currentParticleSystem;
        //--------------------------------------------------------------
        Contour contour;        // User silhouette contour
        //--------------------------------------------------------------
        ofSoundPlayer song;     // Song
        //--------------------------------------------------------------
        Sequence sequence;      // Gestures sequence
        //--------------------------------------------------------------
        ofFbo fbo;
        int fadeAmount;
        bool useFBO;
        //--------------------------------------------------------------
        vector<string> cueList;
        int currentCueIndex;
        //--------------------------------------------------------------
        bool interpolatingWidgets;
        int maxTransitionFrames;
        int nInterpolatedFrames;
        map<ofxUIWidget *, vector<float> > widgetsToUpdate;
        //--------------------------------------------------------------
        bool drawSequence;
        bool drawSequenceSegments;
        bool drawSequencePatterns;
        bool drawSequencePatternsSeparate;
        bool drawMarkers;
        bool drawMarkersPath;
        //--------------------------------------------------------------
        ofxSecondWindow secondWindow;
        //--------------------------------------------------------------
//        vector< vector<ofxUICanvas *> > particleGuis;
        vector<ofxUICanvas *> guis;
        //--------------------------------------------------------------
        ofColor uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo;
        //--------------------------------------------------------------
        ofxUIImageToggle *recordingSequence;  // Toggle to record gestures sequence
        ofxUILabel *songFilename;             // Name of the song
        ofxUILabel *settingsFilename;         // Name of the settings
        ofxUILabel *sequenceFilename;         // Name of the sequence
        ofxUILabel *sequenceDuration;         // Duration of the sequence in seconds
        ofxUILabel *sequenceNumFrames;        // Number of frames of the sequence
        ofxUILabel *sequenceNumMarkers;       // Number of markers of the sequence
        ofxUILabel *cueIndexLabel;            // Current cue index
        ofxUILabel *trackingInfoLabel;        // Information about tracking
        ofxUITextInput *cueName;              // Name of the cue
        ofxUISlider *lowThresh;               // Flocking lower threshold
        ofxUISlider *highThresh;              // Flocking higher threshold
        vector< pair<ofxUILabel *, ofxUIRangeSlider*> > cueSliders; // Cue sliders to assign to long sequence
        //--------------------------------------------------------------
        float dim;                            // Size of GUI elements
        float guiWidth;
        float guiMargin;
        //--------------------------------------------------------------
        float red, green, blue;
        bool bgGradient;
        //--------------------------------------------------------------
        float nearClipping, farClipping;
        float nearThreshold, farThreshold;
        float minContourSize, maxContourSize;
        //--------------------------------------------------------------
        float irThreshold;
        float minMarkerSize, maxMarkerSize;
        float trackerPersistence;
        float trackerMaxDistance;
        //------VMO Declaration-----------------------------------------
        vmo seqVmo;
        vmo::pttr pttrList;
        vmo::belief currentBf;
        vmo::belief prevBf;

        // vector<ofPoint>& loadedFrames;
        int dimensions;
        int numElements;
        float slide;
        float decay;
        vector<float> pastObs;
//        vector<float> pastFeatures;
        vector<float> currentFeatures;
        vector<vector<float> > savedObs;
        vector<vector<float> > vmoObs;
        bool isConv;
        bool initStatus;
        bool isTracking;
        float currentPercent;
        map<int, float> gestureUpdate;
};
