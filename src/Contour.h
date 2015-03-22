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

        void setMinAreaRadius(float minContourSize);
        void setMaxAreaRadius(float maxContourSize);

        //--------------------------------------------------------------
        bool isActive;
        //--------------------------------------------------------------
        int width;
        int height;
        //--------------------------------------------------------------
        float scaleFactor; // scaling factor of the optical flow image
        float flowScale;
        //--------------------------------------------------------------
        float pyrScale;
        int levels;
        int winSize;
        int iterations;
        int polyN;
        float polySigma;
        bool gaussianMode;
        //--------------------------------------------------------------
        float smoothingSize;
        //--------------------------------------------------------------
        vector<ofRectangle> boundingRects;
        vector<ofPolyline> convexHulls;
        vector<ofPolyline> contours;
        //--------------------------------------------------------------
        bool drawBoundingRect;
        bool drawConvexHull;
        bool drawConvexHullLine;
        bool drawContourLine;
        bool drawTangentLines;
        bool drawFlow;

    protected:
    private:
        ofImage rescaled;
        ofRectangle rescaledRect;
        ofxCv::FlowFarneback flow;
        ofxCv::ContourFinder contourFinder;
};
