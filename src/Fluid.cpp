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
    particlesBirthChance         = 0.1;   // 0 ~ 1
    particlesBirthVelocityChance = 0.1;   // 0 ~ 1
    particlesLifetime            = 3.0;   // 0 ~ 10
    particlesLifetimeRnd         = 0.0;   // 0 ~ 1
    particlesMass                = 1.0;   // 0 ~ 2
    particlesMassRnd             = 0.0;   // 0 ~ 1
    particlesSize                = 1.0;   // 0 ~ 10
    particlesSizeRnd             = 0.0;   // 0 ~ 1
    
    // Marker forces
    
    // Input
    markersInput                 = false; // Input are the IR markers?
    contourInput                 = false; // Input is the depth contour?
    markerRadius                 = 50.0;  // Radius of interaction of the markers
    
    // Output
    drawParticles                = true;
    drawVelocity                 = false;
    drawVelocityScalar           = false;
    drawTemperature              = false;
}

void Fluid::setup(int flowWidth, int flowHeight, int drawWidth, int drawHeight, bool doFasterInternalFormat){
    this->width = drawWidth;
    this->height = drawHeight;
    
    // Fluid
    fluid.setup(flowWidth, flowHeight, drawWidth, drawHeight, doFasterInternalFormat);
    
    // Particles
    particleFlow.setup(flowWidth, flowHeight, drawWidth, drawHeight);
    
    // Marker temporal Forces
    numMarkerForces = 3;
    markerForces = new ftDrawForce[numMarkerForces];
    markerForces[0].setup(drawWidth, drawHeight, FT_DENSITY, true);
    markerForces[0].setName("draw full res");
    markerForces[1].setup(flowWidth, flowHeight, FT_VELOCITY, true);
    markerForces[1].setName("draw flow res 1");
    markerForces[2].setup(flowWidth, flowHeight, FT_TEMPERATURE, true);
    markerForces[2].setName("draw flow res 2");
    
    // Visualisation
    displayScalar.allocate(flowWidth, flowHeight);
    velocityField.allocate(flowWidth / 4, flowHeight / 4);
}

void Fluid::update(float dt, vector<irMarker> &markers, Contour &contour){
    
    if(isActive){
        // set fluid parameters
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
            for(unsigned int i = 0; i < markers.size(); i++){
                if (!markers[i].hasDisappeared){
                    for (int i=0; i<numMarkerForces; i++) {
                        if (markerForces[i].getType() == FT_VELOCITY)
                            markerForces[i].setForce(markers[i].velocity);
                        markerForces[i].applyForce(markers[i].smoothPos);
                    }
                }
            }
            updateDrawForces(dt);
        }
        if(contourInput){
            fluid.addVelocity(contour.getOpticalFlowDecay());
            fluid.addDensity(contour.getColorMask());
            fluid.addTemperature(contour.getLuminanceMask());
        }
        
        fluid.update(dt);
        
        if((markersInput || contourInput) && drawParticles){
            // set particle flow parameters
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
            
            if(contourInput) particleFlow.addFlowVelocity(contour.getOpticalFlowDecay());
            particleFlow.addFluidVelocity(fluid.getVelocity());
        }
        particleFlow.update(dt);
    }
}

void Fluid::draw(){
    if(isActive){
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
        if(drawTemperature){
            ofPushStyle();
            ofEnableBlendMode(OF_BLENDMODE_DISABLED);
            displayScalar.setSource(fluid.getTemperature());
            displayScalar.draw(0, 0, width, height);
            ofPopStyle();
        }
        ofPushStyle();
        ofEnableBlendMode(OF_BLENDMODE_ADD);
        ofSetColor(red, green, blue, opacity);
        fluid.draw(0, 0, width, height);
        
        ofEnableBlendMode(OF_BLENDMODE_ADD);
        if (drawParticles){
            particleFlow.draw(0, 0, width, height);
        }
        ofPopStyle();
    }
}

void Fluid::updateDrawForces(float dt){
    for (int i=0; i<numMarkerForces; i++) {
        markerForces[i].update();
        if (markerForces[i].didChange()) {
            // if a force is constant multiply by deltaTime
            float strength = markerForces[i].getStrength();
            if (!markerForces[i].getIsTemporary())
                strength *= dt;
            switch (markerForces[i].getType()) {
                case FT_DENSITY:
                    fluid.addDensity(markerForces[i].getTextureReference(), strength);
                    break;
                case FT_VELOCITY:
                    fluid.addVelocity(markerForces[i].getTextureReference(), strength);
                    particleFlow.addFlowVelocity(markerForces[i].getTextureReference(), strength);
                    break;
                case FT_TEMPERATURE:
                    fluid.addTemperature(markerForces[i].getTextureReference(), strength);
                    break;
                case FT_PRESSURE:
                    fluid.addPressure(markerForces[i].getTextureReference(), strength);
                    break;
                case FT_OBSTACLE:
                    fluid.addTempObstacle(markerForces[i].getTextureReference());
                default:
                    break;
            }
        }
    }
}