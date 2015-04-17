#include "Fluid.h"

Fluid::Fluid(){
    isActive            = false;        // Fluid simulator is active?
    
    opacity             = 255.0;        // Opacity of the fluid
    
    // General properties
    red                 = 255.0;
    green               = 255.0;
    blue                = 255.0;
    
    // Input
    markersInput        = false;        // Input are the IR markers?
    contourInput        = false;        // Input is the depth contour?
    markerRadius        = 50.0;         // Radius of interaction of the markers
    
    // Output
    drawVelocity        = false;
    drawVelocityScalar  = false;
}

void Fluid::setup(int flowWidth, int flowHeight, int drawWidth, int drawHeight, bool doFasterInternalFormat){
    this->width = drawWidth;
    this->height = drawHeight;
    
    // Fluid
    fluid.setup(flowWidth, flowHeight, width, height, doFasterInternalFormat);
    
    // Particles
    particleFlow.setup(flowWidth, flowHeight, width, height);
    
    // Visualisation
    displayScalar.allocate(flowWidth, flowHeight);
    velocityField.allocate(flowWidth / 4, flowHeight / 4);
}

void Fluid::update(float dt, vector<irMarker> &markers, Contour &contour){
    
    if(isActive){
        if(markersInput){
//            for(unsigned int i = 0; i < tempMarkers.size(); i++){
//                if (!tempMarkers[i].hasDisappeared){
//                    ofPoint m = tempMarkers[i].smoothPos;
//                    ofPoint c = ofPoint(640*0.5, 480*0.5) - m;
//        //            fluid.addTemporalForce(m, tempMarkers[i].velocity, ofFloatColor(0.5,0.1,0.0), 10.0f);
//                    c.normalize();
//    //                fluid.addTemporalForce(m, tempMarkers[i].velocity, ofFloatColor(fluidRed, fluidGreen, fluidBlue, fluidOpacity), fluidRadius);
//                }
//            }
        }
        if(contourInput){
            fluid.addVelocity(contour.getOpticalFlowDecay());
            fluid.addDensity(contour.getColorMask());
            fluid.addTemperature(contour.getLuminanceMask());
        }
        
        fluid.update(dt);
        
        if(particlesActive){
            particleFlow.setSpeed(fluid.getSpeed());
            particleFlow.setCellSize(fluid.getCellSize());
            particleFlow.addFlowVelocity(contour.getOpticalFlowDecay());
            particleFlow.addFluidVelocity(fluid.getVelocity());
        }
        particleFlow.update();
    }
}

void Fluid::draw(){
    if(drawVelocity){
        ofPushStyle();
        if (drawVelocityScalar){
            ofEnableBlendMode(OF_BLENDMODE_DISABLED);
            displayScalar.setSource(fluid.getVelocity());
            displayScalar.draw(0, 0, width, height);
        }
        ofEnableBlendMode(OF_BLENDMODE_ADD);
        velocityField.setSource(fluid.getVelocity());
        velocityField.draw(0, 0, width, height);
        ofPopStyle();
    }
    else{
        ofPushStyle();
        ofEnableBlendMode(OF_BLENDMODE_ADD);
        fluid.draw(0, 0, width, height);
        
        ofEnableBlendMode(OF_BLENDMODE_ADD);
        if (particlesActive){
            particleFlow.draw(0, 0, width, height);
        }
        ofPopStyle();
    }
}