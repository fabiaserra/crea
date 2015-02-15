#include "Contour.h"

using namespace ofxCv;
using namespace cv;

Contour::Contour()
{

}

void Contour::setup(){
    smoothingSize = 0;

    drawBoundingRect = false;
    drawConvexHull = true;
    drawConvexHullLine = false;
    drawContourLine = false;
}

void Contour::update(ofxCv::ContourFinder & contourFinder){
    // TODO: compute optical flow
    boundingRects.clear();
    convexHulls.clear();
    contours.clear();

    for(int i = 0; i < contourFinder.size(); i++){

        boundingRects.push_back(toOf(contourFinder.getBoundingRect(i)));

        ofPolyline convexHull;
        convexHull = toOf(contourFinder.getConvexHull(i));
        convexHull = convexHull.getSmoothed(smoothingSize, 0.5);
        convexHulls.push_back(convexHull);

        ofPolyline contour;
        contour = contourFinder.getPolyline(i);
        contour = contour.getSmoothed(smoothingSize, 0.5);
        contours.push_back(contour);
    }
}

void Contour::draw(){
    if(drawBoundingRect){
        ofFill();
        ofSetColor(255);
        for(int i = 0; i < boundingRects.size(); i++)
            ofRect(boundingRects[i]);
    }

    if(drawConvexHull){
        ofSetColor(255);
        for(int i = 0; i < convexHulls.size(); i++){
            ofBeginShape();
            for(int j = 0; j < convexHulls[i].getVertices().size(); j++){
                ofVertex(convexHulls[i].getVertices().at(j).x, convexHulls[i].getVertices().at(j).y);
            }
            ofEndShape();
        }
    }

    if(drawConvexHullLine){
        ofSetColor(255, 0, 0);
        ofSetLineWidth(3);
        for(int i = 0; i < convexHulls.size(); i++)
            convexHulls[i].draw(); //if we only want the contour
    }

    if(drawContourLine){
        ofSetColor(0);
        ofSetLineWidth(3);
        for(int i = 0; i < contours.size(); i++)
            contours[i].draw();
    }
}
