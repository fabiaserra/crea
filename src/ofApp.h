/*
 * Copyright (C) 2018 Fabia Serra Arrizabalaga
 *
 * This file is part of CREA
 *
 * CREA is free software: you can redistribute it and/or modify it under
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
#include "ofFpsCounter.h"
// #include "ofxCv.h"
// #include "ofxKinect.h"
// #include "ofxFlowTools.h"

// Classes
//-----------------------
#include "ParticleSystem.h"
// #include "irMarker.h"
// #include "Contour.h"
// #include "Sequence.h"
// #include "Fluid.h"

// Use the Kinect live input stream
#define KINECT_CONNECTED

// Use an xml IR Markers sequence file as input to Gesture Follower
//#define KINECT_SEQUENCE

// Use a separate window for control interface
#define SECOND_WINDOW
#define PROJECTOR_RESOLUTION_X 640
#define PROJECTOR_RESOLUTION_Y 480

class ofApp : public ofBaseApp
{
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

private:  
	float m_time0; // Time value for computing dt
	
	int m_windowWidth;
	int m_windowHeight;
	
	// Kinect
	// ofxKinect m_kinect;
	
	// std::vector<ofImage> m_savedDepthImages; // the saved Depth images for playback
	// std::vector<ofImage> m_savedIrImages; // the saved IR images for playback
	
	// Sequence m_kinectSequence; // offline sequence for tracking
	
	// bool m_flipKinect; // Flip kinect image
	// bool m_resetKinect; // Reset kinect
	// int m_angle; // Tilt angle of the kinect
	// float m_reScale; // Ratio to scale the Image to full screen
	
	// Input
	int m_numMarkers;
	
	// ofImage m_irImage, m_irOriginal;
	// ofImage m_depthImage, m_depthOriginal;
	// ofImage m_grayThreshNear;
	// ofImage m_grayThreshFar;
	
	// int m_depthNumDilates; // Number of dilates applied to depth image
	// int m_depthNumErodes; // Number of erodes applied to depth image
	// int m_depthBlurValue; // Size of the blur filter to depth image
	
	// int m_irNumDilates; // Number of dilates applied to IR image
	// int m_irNumErodes; // Number of erodes applied to IR image
	// int m_irBlurValue; // Size of the blur filter to IR image
	
	// cv::Mat m_depthCroppingMask;
	// float m_depthLeftMask, m_depthRightMask;
	// float m_depthTopMask, m_depthBottomMask;
	
	// cv::Mat m_irCroppingMask;
	// float m_irLeftMask, m_irRightMask;
	// float m_irTopMask, m_irBottomMask;
	
	// ofxCv::ContourFinder m_irMarkerFinder;
	// ofxCv::RectTrackerFollower<irMarker> m_tracker;
	
	// Contour m_contour; // User silhouette contour
	
	// Fluid
	// Fluid m_fluid;
	
	// Particle systems
	ParticleSystem m_emitterParticles;
    ParticleSystem m_gridParticles;
	// ParticleSystem m_boidsParticles;
	// ParticleSystem m_animationsParticles;
	vector<ParticleSystem> m_particleSystems;
	int m_currentParticleSystem;

	// // Interpolation
	// bool m_interpolatingWidgets;
	// int m_maxTransitionFrames;
	// int m_nInterpolatedFrames;
	
	// // Graphical output
	// bool m_drawMarkers;
	// bool m_drawMarkersPath;
	
	// ofFbo m_fbo;
	// int m_fadeAmount;
	// bool m_useFBO;
	
	// Background
	float m_red, m_green, m_blue;
	bool m_bgGradient;
	
	// float m_nearClipping, m_farClipping;
	// float m_nearThreshold, m_farThreshold;
	// float m_minContourSize, m_maxContourSize;
	
	// float m_irThreshold;
	// float m_minMarkerSize;
	// float m_maxMarkerSize;
	// float m_trackerPersistence;
	// float m_trackerMaxDistance;
};
