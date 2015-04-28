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
        ofPoint getAverageFlowInRegion(ofRectangle rect);
        ofPoint getAverageVelocity();
        ofPoint getVelocityInPoint(ofPoint curPoint);
    
        ofTexture& getOpticalFlowDecay() {return opticalFlow.getOpticalFlowDecay();}
        ofTexture& getLuminanceMask() {return velocityMask.getLuminanceMask();}
        ofTexture& getColorMask() {return velocityMask.getColorMask();} // the same than luminance since we dont have any color in depth img
            
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
        float vMaskStrength;
        int vMaskBlurPasses;
        float vMaskBlurRadius;
        float vMaskRed, vMaskGreen, vMaskBlue;
        float vMaskOpacity;
        ofColor vMaskColor;
        bool vMaskRandomColor;
        //--------------------------------------------------------------
        ofImage previous;
        ofImage diff;
        //--------------------------------------------------------------
        float smoothingSize;
        float lineWidth;
        float scaleContour;
        //--------------------------------------------------------------
        vector<ofRectangle> boundingRects;
        vector<ofPolyline> convexHulls;
        vector<ofPolyline> quads;
        vector<ofPolyline> contours;
        vector<ofPolyline> prevContours;
        vector<ofPolyline> diffContours;
        vector< vector<ofPoint> > velocities;
        //--------------------------------------------------------------
        bool drawBoundingRect;
        bool drawConvexHull;
        bool drawConvexHullLine;
        bool drawContour;
        bool drawContourLine;
        bool drawQuads;
        bool drawTangentLines;
        //--------------------------------------------------------------
        bool drawDiff;
        bool drawDiffImage;
        bool drawFlow;
        bool drawFlowScalar;
        bool drawVelocities;
        //--------------------------------------------------------------

    protected:
        ftOpticalFlow opticalFlow;
        ftVelocityMask velocityMask;
        //--------------------------------------------------------------
        ofxCv::ContourFinder contourFinder;
        ofxCv::ContourFinder contourFinderDiff;
        //--------------------------------------------------------------
        ftDisplayScalar displayScalar;
        ftVelocityField velocityField;
        //--------------------------------------------------------------
        ofRectangle rescaledRect;
        ofTexture flowTexture;
        ofFloatPixels flowPixels;
        ofPixels velocityMaskPixels;
        ofFbo coloredDepthFbo;
        //--------------------------------------------------------------
        void fadeIn(float dt);
        void fadeOut(float dt);
};
