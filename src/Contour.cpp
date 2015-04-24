#include "Contour.h"

using namespace ofxCv;
using namespace cv;

Contour::Contour()
{
    isActive            = false;
    
    // Fading in/out
    isFadingIn          = false; // Opacity fading in?
    isFadingOut         = false; // Opacity fading out?
    startFadeIn         = false; // Fade in has started?
    startFadeOut        = false; // Fade out has started?
    elapsedFadeTime     = 0.0;   // Elapsed time of fade
    fadeTime            = 2.0;   // Transition time of fade
    
    // Color
    red                 = 255.0;
    green               = 255.0;
    blue                = 255.0;
    
    // Opacity
    opacity             = 0.0;   // Actual opacity of the particles
    maxOpacity          = 255.0; // Maximum opacity of particles
    
    doOpticalFlow       = true;  // compute optical flow?

    scaleFactor         = 4.0;   // scaling factor of the depth image to compute the optical flow in lower res.
    flowScale           = 1.5;   // scalar of flow velocities

    // optical flow settings
    flowStrength        = 10.0;  // 0 ~ 100
    flowOffset          = 3.0;   // 1 ~ 10
    flowLambda          = 0.01;  // 0 ~ 0.1
    flowThreshold       = 0.04;  // 0 ~ 0.2
    flowInverseX        = false;
    flowInverseY        = false;
    flowTimeBlurActive  = true;
    flowTimeBlurDecay   = 0.1;   // 0 ~ 1
    flowTimeBlurRadius  = 2.0;   // 0 ~ 10
    
    // velocity mask settings
    vMaskStrength       = 10.0;  // 0 ~ 10
    vMaskBlurPasses     = 1;     // 0 ~ 10
    vMaskBlurRadius     = 5.0;   // 0 ~ 10
    vMaskRed            = 255.0;
    vMaskGreen          = 255.0;
    vMaskBlue           = 255.0;
    vMaskOpacity        = 255.0;
    vMaskColor          = ofColor(vMaskRed, vMaskGreen, vMaskBlue);
    vMaskRandomColor    = false;
    
    // contour settings
    smoothingSize       = 0.0;
    scaleContour        = 1.0;
    lineWidth           = 1.5;

    // graphics output
    drawBoundingRect    = false;
    drawConvexHull      = false;
    drawConvexHullLine  = false;
    drawContour         = false;
    drawContourLine     = false;
    drawQuads           = false;
    drawTangentLines    = false;

    // debug
    drawFlow            = false;
    drawFlowScalar      = false;
    drawDiff            = false;
    drawDiffImage       = false;
    drawVelocities      = false;
}

void Contour::setup(int width, int height){
    this->width     = width;
    this->height    = height;
    
    flowWidth = width/scaleFactor;
    flowHeight = height/scaleFactor;
    
    // Flow & Mask
    opticalFlow.setup(flowWidth, flowHeight);
    opticalFlow.setStrength(100.0);
    velocityMask.setup(width, height);
    
    displayScalar.allocate(flowWidth, flowHeight);
    velocityField.allocate(flowWidth / 4, flowHeight / 4);
//    velocityField.setLineSmooth(true);
    
    rescaledRect.set(0, 0, flowWidth, flowHeight);
    
    contourFinder.setSortBySize(true);  // sort contours by size

    contourFinderDiff.setMinAreaRadius(10);
    contourFinderDiff.setMaxAreaRadius(500);

    // allocate images
    previous.allocate(width, height, OF_IMAGE_GRAYSCALE);
    diff.allocate(width, height, OF_IMAGE_GRAYSCALE);
    
    // allocate flow texture
    flowTexture.allocate(flowWidth, flowHeight, GL_RGB32F);
    flowPixels.allocate(flowWidth, flowHeight, 3);
    velocityMaskPixels.allocate(flowWidth, flowHeight, 4);
    
    // allocate FBO
    coloredDepthFbo.allocate(width, height, GL_RGBA32F);
    
    coloredDepthFbo.begin();
    ofClear(255,255,255, 0);
    coloredDepthFbo.end();
}

void Contour::update(float dt, ofImage &depthImage){

    // absolute difference of previous frame and save it inside diff
    absdiff(previous, depthImage, diff);
    diff.update();

    copy(depthImage, previous);

    if(doOpticalFlow){
        // Compute optical flow
        opticalFlow.setSource(depthImage.getTextureReference());
        opticalFlow.setStrength(flowStrength);
        opticalFlow.setOffset(flowOffset);
        opticalFlow.setLambda(flowLambda);
        opticalFlow.setThreshold(flowThreshold);
        opticalFlow.setInverseY(flowInverseY);
        opticalFlow.setInverseX(flowInverseX);
        opticalFlow.setTimeBlurActive(flowTimeBlurActive);
        opticalFlow.setTimeBlurRadius(flowTimeBlurRadius);
        opticalFlow.setTimeBlurDecay(flowTimeBlurDecay);
        opticalFlow.update(dt);
        
        flowTexture = opticalFlow.getOpticalFlowDecay();
        flowTexture.readToPixels(flowPixels);
        
        coloredDepthFbo.begin();
            ofPushStyle();
            ofClear(255, 255, 255, 0);
            if(vMaskRandomColor){
                vMaskColor.setHsb(ofMap(ofNoise(ofGetElapsedTimef()*0.3), 0.1, 0.9, 0, 255, true), 255, 255);
                vMaskRed = vMaskColor.r;
                vMaskGreen = vMaskColor.g;
                vMaskBlue = vMaskColor.b;
            }
            vMaskColor.set(vMaskRed, vMaskGreen, vMaskBlue, vMaskOpacity);
            ofSetColor(vMaskColor);
            depthImage.draw(0, 0, width, height);
            ofPopStyle();
        coloredDepthFbo.end();
        
        velocityMask.setDensity(coloredDepthFbo.getTextureReference()); // to change mask color
//        velocityMask.setDensity(depthImage.getTextureReference());
        velocityMask.setVelocity(opticalFlow.getOpticalFlow());
        velocityMask.setStrength(vMaskStrength);
        velocityMask.setBlurPasses(vMaskBlurPasses);
        velocityMask.setBlurRadius(vMaskBlurRadius);
        velocityMask.update();
        
        velocityMask.getTextureReference().readToPixels(velocityMaskPixels);
    }
    
    // Contour Finder in the depth Image
    contourFinder.findContours(depthImage);
    
    // Contour Finder in the depth diff Image
    contourFinderDiff.findContours(velocityMaskPixels);
    
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
    
    if(isActive || isFadingOut){
        // if it is the first frame where isActive is true and we are not fading out (trick to fix a bug)
        // start fadeIn and change activeStarted to true so we dont enter anymore
        if(!activeStarted && !isFadingOut){
            activeStarted = true;
            isFadingIn = true;
            isFadingOut = false;
            startFadeIn = true;
            startFadeOut = false;
            opacity = 0.0;
        }
        if(isFadingIn) fadeIn(dt);
        else if(isFadingOut && !isActive) fadeOut(dt);
        else opacity = maxOpacity;
    }
    else if(activeStarted){
        activeStarted = false;
        isFadingIn = false;
        isFadingOut = true;
        startFadeIn = false;
        startFadeOut = true;
    }
}

void Contour::draw(){
    // if is active or we are fading out
    if(isActive || isFadingOut){
        ofPushMatrix();
        ofTranslate(width/2.0, height/2.0);
        ofScale(scaleContour, scaleContour); // Scale it so we can see more contour on the projection
        
        ofPushMatrix();
        ofTranslate(-width/2.0, -height/2.0);
        
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
        
        if(drawContour){
            ofPushStyle();
            ofSetColor(ofColor(red, green, blue), opacity);
            for(int i = 0; i < contours.size(); i++){
                ofBeginShape();
                for(int j = 0; j < contours[i].getVertices().size(); j++){
                    ofVertex(contours[i].getVertices().at(j).x, contours[i].getVertices().at(j).y);
                }
                ofEndShape();
            }
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
        
        if(drawTangentLines){
            ofPushStyle();
            ofSetLineWidth(1);
            ofSetColor(255, 50);
            for(int i = 0; i < contours.size(); i++){
                float numPoints = contours[i].size();
                float tangentLength = 100;
                for(int p = 0; p < 1000; p++){
                    ofPoint point = contours[i].getPointAtPercent(p/1000.0);
                    float floatIndex = p/1000.0 * (numPoints-1);
                    ofVec3f tangent = contours[i].getTangentAtIndexInterpolated(floatIndex) * tangentLength;
                    ofLine(point-tangent/2, point+tangent/2);
                }
            }
            ofPopStyle();
        }
        
        ofPopMatrix();
        ofPopMatrix();
    }
    
    // DEBUGGING DRAWINGS
    if(drawFlow){
        ofPushStyle();
//        ofSetColor(255, 0, 0);
//        ofSetLineWidth(1.2);
//        for (int i = 0; i < width; i+=4){
//            for (int j = 0; j < height; j+=4){
//                ofPoint p = ofPoint(i,j);
//                ofVec2f vel = getFlowOffset(p);
//                ofLine(ofVec2f(i, j), ofVec2f(i, j)+vel);
//            }
//        }
        ofEnableBlendMode(OF_BLENDMODE_ADD);
        velocityField.setSource(opticalFlow.getOpticalFlowDecay());
        velocityField.draw(0, 0, width, height);
        ofPopStyle();
    }
    if(drawFlowScalar){
        ofPushStyle();
        ofEnableBlendMode(OF_BLENDMODE_DISABLED);
        displayScalar.setSource(opticalFlow.getOpticalFlowDecay());
        displayScalar.draw(0, 0, width, height);
        ofPopStyle();
    }
    if(drawDiff){
        ofPushStyle();
        ofSetColor(255, 0, 0);
        ofSetLineWidth(2.5);
        for(int i = 0; i < diffContours.size(); i++){
            diffContours[i].draw();
        }
        ofPopStyle();
    }
    if(drawDiffImage){
        ofPushStyle();
        ofEnableBlendMode(OF_BLENDMODE_DISABLED);
        velocityMask.draw(0, 0, width, height);
        ofPopStyle();
    }
    if(drawVelocities){
        ofPushStyle();
        ofSetColor(255, 0, 0);
        ofSetLineWidth(2);
        for(int i = 0; i < contours.size(); i+=6){
            for(int p = 0; p < contours[i].size(); p++){
                ofLine(contours[i][p], contours[i][p] - getVelocityInPoint(contours[i][p]));
            }
        }
        ofPopStyle();
    }
    prevContours = contours; // Save actual contour to do difference with next one
}

ofVec2f Contour::getFlowOffset(ofPoint p){
    ofPoint p_ = p/scaleFactor;
    ofVec2f offset(0,0);
    
    if(rescaledRect.inside(p_)){
        int x = p_.x;
        int y = p_.y;

        offset.x = flowPixels[(y*flowWidth+x)*3 + 0]; // r
        offset.y = flowPixels[(y*flowWidth+x)*3 + 1]; // g
    }

    return offset*flowScale;
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

void Contour::fadeIn(float dt){
    if(startFadeIn){
        startFadeIn = false;
        elapsedFadeTime = 0.0;
        opacity = 0.0;
    }
    else{
        opacity = ofMap(elapsedFadeTime, 0.0, fadeTime, 0.0, maxOpacity, true);
        elapsedFadeTime += dt;
        if(elapsedFadeTime > fadeTime){
            isFadingIn = false;
            opacity = maxOpacity;
        }
    }
}

void Contour::fadeOut(float dt){
    if(startFadeOut){
        startFadeOut = false;
        elapsedFadeTime = 0.0;
        opacity = maxOpacity;
    }
    else{
        opacity = ofMap(elapsedFadeTime, 0.0, fadeTime, maxOpacity, 0.0, true);
        elapsedFadeTime += dt;
        if(elapsedFadeTime > fadeTime){
            isFadingOut = false;
            opacity = 0.0;
        }
    }
}
