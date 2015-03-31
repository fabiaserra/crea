#pragma once
#include "ofMain.h"
#include "ofxCv.h"

class Contour
{
    public:
        Contour();

        void setup(int width, int height);
        void update(ofImage &depthImage);
        void draw();

        ofPoint getFlowOffset(ofPoint p);
        ofPoint getAverageFlowInRegion(ofRectangle rect);
        ofPoint getAverageVelocity();
        ofPoint getVelocityInPoint(ofPoint curPoint);

        void computeVelocities();
        void setMinAreaRadius(float minContourSize);
        void setMaxAreaRadius(float maxContourSize);

        //--------------------------------------------------------------
        bool isActive;
        //--------------------------------------------------------------
        int width;
        int height;
        //--------------------------------------------------------------
        float opacity;
        //--------------------------------------------------------------
        bool opticalFlow;   // compute optical flow?
        float scaleFactor;  // scaling factor of the optical flow image
        float flowScale;    // scalar to multiply by the offset of flow
        //--------------------------------------------------------------
        float pyrScale;
        int levels;
        int winSize;
        int iterations;
        int polyN;
        float polySigma;
        bool gaussianMode;
        //--------------------------------------------------------------
        ofImage previous;
        ofImage diff;
        //--------------------------------------------------------------
        float smoothingSize;
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
        bool drawDiff;
        bool drawFlow;
        bool drawVelocities;

    protected:
    private:
        ofImage rescaled;
        ofRectangle rescaledRect;
        ofxCv::FlowFarneback flow;
        ofxCv::ContourFinder contourFinder;
        ofxCv::ContourFinder contourFinderDiff;
};
