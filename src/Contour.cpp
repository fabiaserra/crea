#include "Contour.h"

using namespace ofxCv;
using namespace cv;

Contour::Contour()
{
    isActive            = false;
    
    red                 = 255.0;
    green               = 255.0;
    blue                = 255.0;
    
    opacity             = 255.0;
    
    doOpticalFlow       = true; // compute optical flow?

    scaleFactor         = 4.0;  // scaling factor of the depth image to compute the optical flow in lower res.
    flowScale           = 0.1;  // scalar of flow velocities

    // optical flow settings
    pyrScale            = 0.5;  // 0~1
    levels              = 4;    // 1~8
    winSize             = 8;    // 4~64
    iterations          = 2;    // 1~8
    polyN               = 7;    // 5~10
    polySigma           = 1.5;  // 1.1~2
    gaussianMode        = false;
    
    smoothingSize       = 0.0;
    scaleContour        = 1.0;
    lineWidth           = 1.5;

    // graphics output
    drawBoundingRect    = false;
    drawConvexHull      = false;
    drawConvexHullLine  = false;
    drawContourLine     = false;
    drawQuads           = false;
    drawTangentLines    = false;

    // debug
    drawDiff            = false;
    drawFlow            = false;
    drawFlowScalar      = true;
    drawFlowField       = true;
    drawVelocities      = false;
}

void Contour::setup(int width, int height){
    this->width     = width;
    this->height    = height;
    
    flowWidth = width/scaleFactor;
    flowHeight = height/scaleFactor;
    
    // Flow & Mask
    opticalFlow.setup(flowWidth, flowHeight);
    opticalFlow.setStrength(30.0);
    velocityMask.setup(width, height);
    
    displayScalar.allocate(flowWidth, flowHeight);
    velocityField.allocate(flowWidth / 4, flowHeight / 4);
    
//    rescaled.allocate(flowWidth, flowHeight, OF_IMAGE_GRAYSCALE);
//    rescaledRect.set(0, 0, rescaled.width, rescaled.height);
    
    contourFinder.setSortBySize(true);  // sort contours by size

    contourFinderDiff.setMinAreaRadius(10);
    contourFinderDiff.setMaxAreaRadius(500);

    // allocate images
    previous.allocate(width, height, OF_IMAGE_GRAYSCALE);
    diff.allocate(width, height, OF_IMAGE_GRAYSCALE);
    
    // allocate flow texture
    flowTexture.allocate(flowWidth, flowHeight, GL_RGB32F);
    flowPixels.allocate(flowWidth, flowHeight, 3);
}

void Contour::update(float dt, ofImage &depthImage){

    if(isActive){

        // absolute difference of previous frame and save it inside diff
        absdiff(previous, depthImage, diff);
        diff.update();

        copy(depthImage, previous);

        if(doOpticalFlow){
//            resize(depthImage, rescaled);
//            rescaled.update();

//            // Compute optical flow
//            flow.setPyramidScale(pyrScale);
//            flow.setNumLevels(levels);
//            flow.setWindowSize(winSize);
//            flow.setNumIterations(iterations);
//            flow.setPolyN(polyN);
//            flow.setPolySigma(polySigma);
//            flow.setUseGaussian(gaussianMode);
//            flow.calcOpticalFlow(rescaled); // optical flow on rescaled depth image
            
            opticalFlow.setSource(depthImage.getTextureReference());
            opticalFlow.update(dt);
            
            velocityMask.setDensity(depthImage.getTextureReference());
            velocityMask.setVelocity(opticalFlow.getOpticalFlow());
            velocityMask.update();
        }
        
        // Contour Finder in the depth Image
        contourFinder.findContours(depthImage);

        // Contour Finder in the depth diff Image
        contourFinderDiff.findContours(diff);

        int n = contourFinder.size();
        int m = contourFinderDiff.size();

        // Clear vectors
        boundingRects.clear();
        convexHulls.clear();
        contours.clear();
        quads.clear();
        diffContours.clear();

        // Initialize vectors
        boundingRects.resize(n);
        convexHulls.resize(n);
        contours.resize(n);
        quads.resize(n);
        diffContours.resize(m);

        for(int i = 0; i < n; i++){
            boundingRects[i] = toOf(contourFinder.getBoundingRect(i));

            ofPolyline convexHull;
            convexHull = toOf(contourFinder.getConvexHull(i));
            convexHulls[i] = convexHull;

            ofPolyline contour;
            contour = contourFinder.getPolyline(i);
            contour = contour.getSmoothed(smoothingSize, 0.5);
            contours[i] = contour;

            ofPolyline quad;
            quad = toOf(contourFinder.getFitQuad(i));
            quads[i] = quad;
        }

        
        for(int i = 0; i < m; i++){
            ofPolyline diffContour;
            diffContour = contourFinderDiff.getPolyline(i);
            diffContours[i] = diffContour;
        }

//        // Compute velocities on all the contour points
//        if(prevContours.size() > 0) computeVelocities();

    }
}

void Contour::draw(){
    if(isActive){
//        ofPushMatrix();
//        ofTranslate(width/2.0, height/2.0);
//        ofScale(scaleContour, scaleContour); // Scale it so we can see more contour on the projection
//        
//        ofPushMatrix();
//        ofTranslate(-width/2.0, -height/2.0);
        
        if(drawBoundingRect){
            ofPushStyle();
            ofFill();
            ofSetColor(ofColor(red, green, blue), opacity);
            for(int i = 0; i < boundingRects.size(); i++)
                ofRect(boundingRects[i]);
            ofPopStyle();
        }

        if(drawConvexHull){
            ofPushStyle();
            ofFill();
            ofSetColor(ofColor(red, green, blue), opacity);
            for(int i = 0; i < convexHulls.size(); i++){
                ofBeginShape();
                for(int j = 0; j < convexHulls[i].getVertices().size(); j++){
                    ofVertex(convexHulls[i].getVertices().at(j).x, convexHulls[i].getVertices().at(j).y);
                }
                ofEndShape();
            }
            ofPopStyle();
        }

        if(drawConvexHullLine){
            ofPushStyle();
            ofSetColor(ofColor(red, green, blue), opacity);
            ofSetLineWidth(lineWidth);
            for(int i = 0; i < convexHulls.size(); i++)
                convexHulls[i].draw(); //if we only want the contour
            ofPopStyle();
        }

        if(drawContourLine){
            ofPushStyle();
            ofSetColor(ofColor(red, green, blue), opacity);
            ofSetLineWidth(lineWidth);
            for(int i = 0; i < contours.size(); i++){
//                if(i == 0) ofSetColor(255, 0, 0);
//                else if(i == 1) ofSetColor(0, 255, 0);
//                else ofSetColor(0, 0, 255);
                contours[i].draw();
            }
            ofPopStyle();
        }

        if(drawQuads){
            ofPushStyle();
            ofSetColor(ofColor(red, green, blue), opacity);
            ofSetLineWidth(lineWidth);
            for(int i = 0; i < quads.size(); i++){
                quads[i].draw();
            }
            ofPopStyle();
        }
        
//        if(drawTangentLines){
//            ofPushStyle();
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
//            ofPopStyle();
//        }
        
//        ofPopMatrix();
//        ofPopMatrix();
        
        // DEBUGGING DRAWINGS

        if(drawDiff){
            ofPushStyle();
            ofSetColor(255);
//            diff.draw(0, 0);
//            ofSetColor(255, 0, 0);
//            ofSetLineWidth(2);
//            for(int i = 0; i < diffContours.size(); i++){
//                diffContours[i].draw();
//            }
            ofEnableBlendMode(OF_BLENDMODE_DISABLED);
            velocityMask.draw(0, 0, width, height);
            ofPopStyle();
        }

        if(drawFlow){
            ofPushStyle();
//            ofSetColor(255);
//            rescaled.draw(0, 0, width, height);
//            ofSetColor(255, 0, 0);
//            ofSetLineWidth(1.5);
//            flow.draw(0, 0, width, height);
            
            opticalFlow.getOpticalFlow().draw(0, 0, width, height);
            if(drawFlowScalar){
                ofEnableBlendMode(OF_BLENDMODE_DISABLED);
                displayScalar.setSource(opticalFlow.getOpticalFlowDecay());
                displayScalar.draw(0, 0, width, height);
            }
            if(drawFlowField){
                ofEnableBlendMode(OF_BLENDMODE_ADD);
                velocityField.setSource(opticalFlow.getOpticalFlowDecay());
                velocityField.draw(0, 0, width, height);
            }
            ofPopStyle();
        }

        if(drawVelocities){
            ofPushStyle();
            ofSetColor(255, 0, 0);
            ofSetLineWidth(1);
            for(int i = 0; i < contours.size(); i++){
                for(int p = 0; p < contours[i].size(); p++){
                    ofLine(contours[i][p], contours[i][p] - getVelocityInPoint(contours[i][p]));
                }
            }
            ofPopStyle();
        }
        
        prevContours = contours; // Save actual contour to do difference with next one
    }
}

ofPoint Contour::getFlowOffset(ofPoint p){
//    ofPoint p_ = p/scaleFactor;
//
//    if(rescaledRect.inside(p_)){
//        return flow.getFlowOffset(p_.x, p_.y) * flowScale;
//    }
//    else return ofPoint(0, 0);
    
    ofVec2f* flowVectors = opticalFlow.getFlowVectors();
    int idx = p.x + p.y*flowWidth;
    ofVec2f offset(0, 0);
    if(idx < opticalFlow.getFlowVectorSize()) offset = *(flowVectors + idx);
    return offset;
}

ofPoint Contour::getAverageFlowInRegion(ofRectangle rect){
    rect.scale(1.0/scaleFactor);
    rect.setPosition(rect.getPosition()/scaleFactor);

    if(rescaledRect.inside(rect)){
        return flow.getAverageFlowInRegion(rect) * flowScale;
    }
    else return ofPoint(0, 0);
}


ofPoint Contour::getAverageVelocity(){
    return flow.getAverageFlow();
}

ofTexture& Contour::getFlowTexture(){
    if(flow.getWidth() > 0){
        int w = flow.getWidth();
        int h = flow.getHeight();
        
        for(int y = 0; y < h; y ++) {
            for(int x = 0; x < w; x ++) {
                ofPoint v = flow.getFlowOffset(x, y);
                flowPixels[(y*w+x)*3 + 0] = v.x;	// r
                flowPixels[(y*w+x)*3 + 1] = v.y;	// g
                flowPixels[(y*w+x)*3 + 2] = 0.0;    // b
            }
        }
//        toOf(flow.getFlow(), flowPixels); // maybe faster but doesn't work as expected
        flowTexture.loadData(flowPixels);
    }
    return flowTexture;
}

void Contour::computeVelocities(){
    // Get velocities from prev frame to current frame
//    int numContours = MIN(contours.size(), prevContours.size());
    velocities.clear();
    for(int i = 0; i < contours.size(); i++){
        vector<ofPoint> contourVelocities;
        for(int p = 0; p < contours[i].size(); p++){
            ofPoint curPoint = contours[i][p];
            ofPoint prevPoint = prevContours[i].getClosestPoint(curPoint);
            ofPoint vel(0, 0);
            if(prevPoint.squareDistance(curPoint) < 200){
                vel = curPoint - prevPoint;
            }
            contourVelocities.push_back(vel);
        }
        velocities.push_back(contourVelocities);
    }
}

ofPoint Contour::getVelocityInPoint(ofPoint curPoint){
    // Get velocity in point from closest point in prev frame
    float minDistSqrd = 600;
    ofPoint vel(0, 0);
    for(int i = 0; i < prevContours.size(); i++){
        ofPoint prevPoint = prevContours[i].getClosestPoint(curPoint);
        float distSqrd = prevPoint.squareDistance(curPoint);
        if(distSqrd < minDistSqrd){
            minDistSqrd = distSqrd;
            vel = curPoint - prevPoint;
        }
    }
    return vel;
}

void Contour::setMinAreaRadius(float minContourSize){
    contourFinder.setMinAreaRadius(minContourSize);
}

void Contour::setMaxAreaRadius(float maxContourSize){
    contourFinder.setMaxAreaRadius(maxContourSize);
}
