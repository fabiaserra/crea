/*
 * Copyright (C) 2015 Fabia Serra Arrizabalaga
 *
 * This file is part of Crea
 *
 * Crea is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the Free
 * Software Foundation (FSF), either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the Affero GNU General Public License
 * version 3 along with this program.  If not, see http://www.gnu.org/licenses/
 */

#pragma once

// Addons
//-----------------------
#include "ofMain.h"
#include "ofxUI.h"
#include "ofxCv.h"
#include "ofxKinect.h"
#include "ofxSecondWindow.h"
#include "ofxFlowTools.h"

// Classes
//-----------------------
#include "ParticleSystem.h"
#include "irMarker.h"
#include "Contour.h"
#include "Sequence.h"
#include "Fluid.h"

// VMO files
//-----------------------
// include gesture follower files (you need to have vmo.cpp, vmo.h, helper.cpp and helper.h in src)
//#define GESTURE_FOLLOWER
#ifdef GESTURE_FOLLOWER
#include "vmo.h"
#include "helper.h"
#endif

// Use the Kinect live input stream
//#define KINECT_CONNECTED

// Use an xml IR Markers sequence file as input to Gesture Follower
//#define KINECT_SEQUENCE

// Use a separate window for control interface
#define SECOND_WINDOW
#define PROJECTOR_RESOLUTION_X 640
#define PROJECTOR_RESOLUTION_Y 480

class ofApp : public ofBaseApp{
    public:
        void setup();
        void update();
        void draw();
    
        void setupGUI();    
        void setupHelperGUI();
        void setupBasicsGUI();
        void setupKinectGUI();
        void setupGesturesGUI();
        void setupCueListGUI();
        void setupOpticalFlowGUI();
        void setupFluidSolverGUI();
        void setupFluidExtrasGUI();
        void setupContourGUI();
        void setupEmitterGUI();
        void setupGridGUI();
        void setupBoidsGUI();
        void setupAnimationsGUI();

        void saveGUISettings(const string path, const bool isCue);
        void loadGUISettings(const string path, const bool isCue, const bool interpolate);
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
    
        void resetCueSliders();
    
        //--------------------------------------------------------------
        float time0;            // Time value for computing dt
        //--------------------------------------------------------------
        ofxKinect kinect;
        //--------------------------------------------------------------
        int windowWidth;
        int windowHeight;
        //--------------------------------------------------------------
        vector<ofImage *> savedDepthImages; // the saved Depth images for playback
        vector<ofImage *> savedIrImages;    // the saved IR images for playback
        //--------------------------------------------------------------
        Sequence kinectSequence;            // offline sequence for vmo tracking
        //--------------------------------------------------------------
        bool  flipKinect;       // Flip kinect image
        bool  resetKinect;      // Reset kinect
        int   angle;            // Tilt angle of the kinect
        float reScale;          // Ratio to scale the Image to full screen
        //--------------------------------------------------------------
        int depthNumDilates;    // Number of dilates applied to depth image
        int depthNumErodes;     // Number of erodes applied to depth image
        int depthBlurValue;     // Size of the blur filter to depth image
        //--------------------------------------------------------------
        int irNumDilates;       // Number of dilates applied to IR image
        int irNumErodes;        // Number of erodes applied to IR image
        int irBlurValue;        // Size of the blur filter to IR image
        //--------------------------------------------------------------
        ofImage irImage, irOriginal;
        ofImage depthImage, depthOriginal;
        ofImage grayThreshNear;
        ofImage grayThreshFar;
        //--------------------------------------------------------------
        cv::Mat depthCroppingMask;
        float depthLeftMask, depthRightMask;
        float depthTopMask, depthBottomMask;
        //--------------------------------------------------------------
        cv::Mat irCroppingMask;
        float irLeftMask, irRightMask;
        float irTopMask, irBottomMask;
        //--------------------------------------------------------------
        ofxCv::ContourFinder irMarkerFinder;
        ofxCv::RectTrackerFollower<irMarker> tracker;
        //--------------------------------------------------------------
        int numMarkers;
        //--------------------------------------------------------------
        Contour contour;        // User silhouette contour
        //--------------------------------------------------------------
        Fluid fluid;
        //--------------------------------------------------------------
        ParticleSystem *emitterParticles;
        ParticleSystem *gridParticles;
        ParticleSystem *boidsParticles;
        ParticleSystem *animationsParticles;
        vector<ParticleSystem *> particleSystems;
        int currentParticleSystem;
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
        vector<ofxUICanvas *> guis;
        //--------------------------------------------------------------
        ofColor uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo;
        //--------------------------------------------------------------
        vector< ofxUILabel *> labelTabs;      // Labels of the main menu tabs
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
//        ofxUISortableList *cueSortableList;   // Sortable list to see all the cues and be able to reorder them
        vector< pair<ofxUILabel *, ofxUIRangeSlider*> > cueSliders; // Cue sliders to assign to long sequence
        //--------------------------------------------------------------
        float dim;                            // Size of GUI elements
        float guiWidth;
        float guiMargin;
        //--------------------------------------------------------------
//        shared_ptr<GuiApp> gui;
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
        #ifdef GESTURE_FOLLOWER
        vmo seqVmo;
        vmo::pttr pttrList;
        vmo::belief currentBf;
        vmo::belief prevBf;
        #endif

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
