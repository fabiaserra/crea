#include "Fluid.h"

Fluid::Fluid(){
    isActive                     = false;        // Fluid simulator is active?
    
    opacity                      = 255.0;        // Opacity of the fluid
    
    // General properties
    red                          = 255.0;
    green                        = 255.0;
    blue                         = 255.0;
    
    // Fluid parameters
    speed                        = 10.0;  // 0 ~ 100
    cellSize                     = 1.25;  // 0 ~ 2
    numJacobiIterations          = 40;    // 1 ~ 100
    viscosity                    = 0.005; // 0 ~ 1
    vorticity                    = 0.1;   // 0 ~ 1
    dissipation                  = 0.01;  // 0 ~ 0.02
    dissipationVelocityOffset    = 0.0;   // -0.01 ~ 0.01
    dissipationDensityOffset     = 0.0;   // -0.01 ~ 0.01
    dissipationTemperatureOffset = 0.0;   // -0.01 ~ 0.01
    smokeSigma                   = 0.05;  // 0 ~ 1
    smokeWeight                  = 0.05;  // 0 ~ 1
    ambientTemperature           = 0.0;   // 0 ~ 1
    gravity                      = ofPoint(0,0); // ofVec2f(-1,-1) ~ ofVec2f(1,1)
    clampForce                   = 0.0;   // 0 ~ 0.1
    maxVelocity                  = 1.0;   // 0 ~ 5
    maxDensity                   = 1.0;   // 0 ~ 10
    maxTemperature               = 1.0;   // 0 ~ 5
    densityFromVorticity         = 0.0;   // -0.1 ~ 0.1
    densityFromPressure          = 0.0;   // -0.5 ~ 0.5
    
    // Particle flow parameters
    isParticlesActive            = true;
    particlesBirthChance         = 0.1;   // 0 ~ 1
    particlesBirthVelocityChance = 0.1;   // 0 ~ 1
    particlesLifetime            = 3.0;   // 0 ~ 10
    particlesLifetimeRnd         = 0.0;   // 0 ~ 1
    particlesMass                = 1.0;   // 0 ~ 2
    particlesMassRnd             = 0.0;   // 0 ~ 1
    particlesSize                = 1.0;   // 0 ~ 10
    particlesSizeRnd             = 0.0;   // 0 ~ 1
    
    // Input
    markersInput                 = false; // Input are the IR markers?
    contourInput                 = false; // Input is the depth contour?
    markerRadius                 = 50.0;  // Radius of interaction of the markers
    
    // Output
    drawVelocity                 = false;
    drawVelocityScalar           = false;
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
        
        fluid.setSpeed(speed);
        fluid.setCellSize(cellSize);
        fluid.setNumJacobiIterations(numJacobiIterations);
        fluid.setViscosity(viscosity);
        fluid.setVorticity(vorticity);
        fluid.setDissipation(dissipation);
        fluid.setDissipationVelocityOffset(dissipationVelocityOffset);
        fluid.setDissipationDensityOffset(dissipationDensityOffset);
        fluid.setDissipationTemperatureOffset(dissipationTemperatureOffset);
        fluid.setSmokeSigma(smokeSigma);
        fluid.setSmokeWeight(smokeWeight);
        fluid.setAmbientTemperature(ambientTemperature);
        fluid.setGravity(gravity);
        fluid.setClampForce(clampForce);
        fluid.setMaxVelocity(maxVelocity);
        fluid.setMaxDensity(maxDensity);
        fluid.setMaxTemperature(maxTemperature);
        fluid.setDensityFromVorticity(densityFromVorticity);
        fluid.setDensityFromPressure(densityFromPressure);

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
        
        if(isParticlesActive){
            particleFlow.setSpeed(fluid.getSpeed());
            particleFlow.setCellSize(fluid.getCellSize());
            particleFlow.setBirthChance(particlesBirthChance);
            particleFlow.setBirthVelocityChance(particlesBirthVelocityChance);
            particleFlow.setLifeSpan(particlesLifetime);
            particleFlow.setLifeSpanSpread(particlesLifetimeRnd);
            particleFlow.setMass(particlesMass);
            particleFlow.setMassSpread(particlesMassRnd);
            particleFlow.setSize(particlesSize);
            particleFlow.setSizeSpread(particlesSizeRnd);
            
            particleFlow.addFlowVelocity(contour.getOpticalFlowDecay());
            particleFlow.addFluidVelocity(fluid.getVelocity());
        }
        particleFlow.update(dt);
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
        if (isParticlesActive){
            particleFlow.draw(0, 0, width, height);
        }
        ofPopStyle();
    }
}