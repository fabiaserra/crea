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
#include "ofMain.h"
#include "ofxCv.h"
#include "ofxFlowTools.h"

using namespace flowTools;

class Contour{
    public:
        Contour();

        void setup(int width, int height, float scaleFactor = 4.0);
        void update(float dt, ofImage &depthImage);
        void draw();

        ofVec2f getFlowOffset(ofPoint p);
        ofVec2f getAverageFlow();
        ofVec2f getVelocityInPoint(ofPoint curPoint);
    
        ofTexture& getOpticalFlowDecay() {return opticalFlow.getOpticalFlowDecay();}
        ofTexture& getLuminanceMask() {return velocityMask.getLuminanceMask();}
        ofTexture& getColorMask() {return velocityMask.getColorMask();}
            
        float getFlowWidth() {return flowWidth;}
        float getFlowHeight() {return flowHeight;}

        void computeVelocities();
        void setMinAreaRadius(float minContourSize) {contourFinder.setMinAreaRadius(minContourSize);}
        void setMaxAreaRadius(float maxContourSize) {contourFinder.setMaxAreaRadius(maxContourSize);}

        //--------------------------------------------------------------
        bool isActive;
        //--------------------------------------------------------------
        bool doFading;          // Do opacity fading?
        bool activeStarted;     // Active has started?
        bool isFadingIn;        // Opacity fading in?
        bool isFadingOut;       // Opacity fading out?
        bool startFadeIn;       // Fade in has started?
        bool startFadeOut;      // Fade out has started?
        float elapsedFadeTime;  // Elapsed time of fade
        float fadeTime;         // Transition time of fade
        //--------------------------------------------------------------
        int width;
        int height;
        //--------------------------------------------------------------
        int flowWidth;
        int flowHeight;
        //--------------------------------------------------------------
        float opacity;
        float maxOpacity;
        //--------------------------------------------------------------
        float red, green, blue;     // Color of the contour
        //--------------------------------------------------------------
        bool doOpticalFlow;     // compute optical flow?
        float scaleFactor;      // scaling factor of the optical flow image
        //--------------------------------------------------------------
        float flowStrength;
        int   flowOffset;
        float flowLambda;
        float flowThreshold;
        bool  flowInverseX;
        bool  flowInverseY;
        bool  flowTimeBlurActive;
        float flowTimeBlurDecay;
        float flowTimeBlurRadius;
        //--------------------------------------------------------------
//        float vMaskStrength;
        int vMaskBlurPasses;
        float vMaskBlurRadius;
        float vMaskRed, vMaskGreen, vMaskBlue;
        float vMaskOpacity;
        ofColor vMaskColor;
        bool vMaskRandomColor;
        //--------------------------------------------------------------
//        ofImage previous;
//        ofImage diff;
        //--------------------------------------------------------------
        float smoothingSize;
        float lineWidth;
        float scaleContour;
        //--------------------------------------------------------------
        vector<ofRectangle> boundingRects;
        vector<ofPolyline> convexHulls;
        vector<ofPolyline> contours;    // silhouettes
        vector<ofPolyline> prevContours;
        vector<ofPolyline> diffContours;
        vector<ofPolyline> vMaskContours;
        vector< vector<ofPoint> > velocities;
        //--------------------------------------------------------------
        bool drawBoundingRect;
        bool drawBoundingRectLine;
        bool drawConvexHull;
        bool drawConvexHullLine;
        bool drawSilhouette;
        bool drawSilhouetteLine;
        bool drawTangentLines;
        //--------------------------------------------------------------
        bool drawFlow;
        bool drawFlowScalar;
        bool drawVelocities;
        bool drawVelMask;
        bool drawVelMaskContour;
        //--------------------------------------------------------------

    protected:
        ftOpticalFlow opticalFlow;
        ftVelocityMask velocityMask;
        //--------------------------------------------------------------
        ofxCv::ContourFinder contourFinder; 
        ofxCv::ContourFinder contourFinderVelMask;
//        ofxCv::ContourFinder contourFinderDiff;
        //--------------------------------------------------------------
        ftDisplayScalar displayScalar;
        ftVelocityField velocityField;
        //--------------------------------------------------------------
        ofRectangle rescaledRect;
        ofFbo flowFbo;
        ofFloatPixels flowPixels;
        ofFbo velocityMaskFbo;
        ofPixels velocityMaskPixels;
        ofFbo coloredDepthFbo;
        //--------------------------------------------------------------
        void fadeIn(float dt);
        void fadeOut(float dt);
};
