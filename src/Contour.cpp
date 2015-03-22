#include "Contour.h"

using namespace ofxCv;
using namespace cv;

Contour::Contour()
{
    isActive = false;
}

void Contour::setup(int width, int height){
    smoothingSize = 0;

    this->width     = width;
    this->height    = height;

    scaleFactor = 4.0;
    flowScale = 0.1;
    rescaled.allocate((float)width/scaleFactor, (float)height/scaleFactor, OF_IMAGE_GRAYSCALE);
    rescaledRect.set(0, 0, rescaled.width, rescaled.height);

    pyrScale        = 0.5;  // 0~1
    levels          = 4;    // 1~8
    winSize         = 8;    // 4~64
    iterations      = 2;    // 1~8
    polyN           = 7;    // 5~10
    polySigma       = 1.5;  // 1.1~2
    gaussianMode    = false;

    drawBoundingRect    = false;
    drawConvexHull      = false;
    drawConvexHullLine  = false;
    drawContourLine     = false;
    drawFlow            = false;
    drawTangentLines    = false;
}

void Contour::update(ofImage &depthImage){

    if(isActive){
        resize(depthImage, rescaled);
        rescaled.update();

        // Compute optical flow
        flow.setPyramidScale(pyrScale);
        flow.setNumLevels(levels);
        flow.setWindowSize(winSize);
        flow.setNumIterations(iterations);
        flow.setPolyN(polyN);
        flow.setPolySigma(polySigma);
        flow.setUseGaussian(gaussianMode);
        flow.calcOpticalFlow(rescaled); // optical flow on rescaled depth image

    //    int blurAmount = 21;
    //    blur(flow, blurAmount);

        // Contour Finder in the depth Image
        contourFinder.findContours(depthImage);

        boundingRects.clear();
        convexHulls.clear();
        contours.clear();

        for(int i = 0; i < contourFinder.size(); i++){

            boundingRects.push_back(toOf(contourFinder.getBoundingRect(i)));

            ofPolyline convexHull;
            convexHull = toOf(contourFinder.getConvexHull(i));
            convexHulls.push_back(convexHull);

            ofPolyline contour;
            contour = contourFinder.getPolyline(i);
            contour = contour.getSmoothed(smoothingSize, 0.5);
            contours.push_back(contour);
        }
    }
}

void Contour::draw(){
    if(isActive){
        ofPushStyle();
        if(drawBoundingRect){
            ofFill();
            ofSetColor(255);
            for(int i = 0; i < boundingRects.size(); i++)
                ofRect(boundingRects[i]);
        }

        if(drawConvexHull){
            ofFill();
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

        if(drawFlow){
//            ofSetColor(255);
//            rescaled.draw(0, 0, width, height);
            ofSetColor(255, 0, 0);
            flow.draw(0, 0, width, height);
        }

//        if(drawTangentLines){
//            ofSetLineWidth(1);
//            ofSetColor(255, 50);
//            for(int i = 0; i < contours.size(); i++){
//                float numPoints = contours[i].size();
//                float tangentLength = 100;
//                for(int p = 0; p < 1000; p++){
//                    ofPoint point = contours[i].getPointAtPercent(p/1000.0);
//                    float floatIndex = p/1000.0 * (numPoints-1);
//                    ofVec3f tangent = contours[i].getTangentAtIndexInterpolated(floatIndex) * tangentLength;
//                    ofLine(point-tangent/2, point+tangent/2);
//                }
//            }
//        }

        ofPopStyle();
    }
}

ofPoint Contour::getFlowOffset(ofPoint p){
    ofPoint p_ = p/scaleFactor;

    if(rescaledRect.inside(p_)){
        return flow.getFlowOffset(p_.x, p_.y) * flowScale;
    }
    else return ofPoint(0, 0);
}

ofPoint Contour::getAverageFlowInRegion(ofRectangle rect){
    rect.scale(1.0/scaleFactor);

    if(rescaledRect.inside(rect)){
        return flow.getAverageFlowInRegion(rect) * flowScale;
    }
    else return ofPoint(0, 0);
}


ofPoint Contour::getAverageVelocity(){
    return flow.getAverageFlow();
}

void Contour::setMinAreaRadius(float minContourSize){
    contourFinder.setMinAreaRadius(minContourSize);
}

void Contour::setMaxAreaRadius(float maxContourSize){
    contourFinder.setMaxAreaRadius(maxContourSize);
}

