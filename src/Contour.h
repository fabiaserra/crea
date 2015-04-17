#pragma once
#include "ofMain.h"
#include "ofxCv.h"
#include "ofxFlowTools.h"

using namespace flowTools;

class Contour{
    public:
        Contour();

        void setup(int width, int height);
        void update(float dt, ofImage &depthImage);
        void draw();

        ofVec2f getFlowOffset(ofPoint p);
        ofPoint getAverageFlowInRegion(ofRectangle rect);
        ofPoint getAverageVelocity();
        ofPoint getVelocityInPoint(ofPoint curPoint);
    
        ofTexture getFlowTexture() {return flowTexture;}

        void computeVelocities();
        void setMinAreaRadius(float minContourSize) {contourFinder.setMinAreaRadius(minContourSize);}
        void setMaxAreaRadius(float maxContourSize) {contourFinder.setMaxAreaRadius(maxContourSize);}

        //--------------------------------------------------------------
        bool isActive;
        //--------------------------------------------------------------
        int width;
        int height;
        //--------------------------------------------------------------
        int flowWidth;
        int flowHeight;
        //--------------------------------------------------------------
        float opacity;
        //--------------------------------------------------------------
        float red, green, blue;     // Color of the contour
        //--------------------------------------------------------------
        bool doOpticalFlow;   // compute optical flow?
        float scaleFactor;  // scaling factor of the optical flow image
        float flowScale;    // scalar to multiply by the offset of flow
        //--------------------------------------------------------------
        ftOpticalFlow opticalFlow;
        ftVelocityMask velocityMask;
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
        bool drawContourLine;
        bool drawQuads;
        bool drawTangentLines;
        //--------------------------------------------------------------
        bool drawDiff;
        bool drawFlow;
        bool drawFlowScalar;
        bool drawFlowField;
        bool drawVelocities;
    
    protected:
    private:
        ofxCv::ContourFinder contourFinder;
        ofxCv::ContourFinder contourFinderDiff;
        ftDisplayScalar displayScalar;
        ftVelocityField velocityField;
        ofRectangle rescaledRect;
        ofTexture flowTexture;
        ofFloatPixels flowPixels;
};
