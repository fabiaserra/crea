#pragma once
#include "ofMain.h"
#include "ofxCv.h"

class Contour
{
    public:
        Contour();

        void setup();
        void update(ofxCv::ContourFinder & contourFinder);
        void draw();

        vector<ofRectangle> boundingRects;
        vector<ofPolyline> convexHulls;
        vector<ofPolyline> contours;

        float smoothingSize;

        bool isActive;

        bool drawBoundingRect;
        bool drawConvexHull;
        bool drawConvexHullLine;
        bool drawContourLine;
        bool drawTangentLines;

    protected:
    private:
};
