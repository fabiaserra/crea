/*
 * Copyright (C) 2015 Fabia Serra Arrizabalaga
 *
 * This file is part of Crea
 *
 * Crea is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the Free
 * Software Foundation (FSF), either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the Affero GNU General Public License
 * version 3 along with this program.  If not, see http://www.gnu.org/licenses/
 */

#include "Fluid.h"

Fluid::Fluid(){
    isActive                     = false; // Fluid simulator is active?
    particlesActive              = false; // Fluid particles are active?
    
    // Fading in/out
    isFadingIn                   = false; // Opacity fading in?
    isFadingOut                  = false; // Opacity fading out?
    startFadeIn                  = false; // Fade in has started?
    startFadeOut                 = false; // Fade out has started?
    elapsedFadeTime              = 0.0;   // Elapsed time of fade
    fadeTime                     = 1.2;   // Transition time of fade
    
    // Color properties
    red                          = 255.0;
    green                        = 255.0;
    blue                         = 255.0;
    opacity                      = 0.0;   // Actual general opacity of the fluid
    maxOpacity                   = 255.0; // Maximum general opacity of the fluid
    
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
    
    // Fluid particles parameters
    particlesBirthChance         = 0.1;   // 0 ~ 1
    particlesBirthVelocityChance = 0.1;   // 0 ~ 1
    particlesLifetime            = 3.0;   // 0 ~ 10
    particlesLifetimeRnd         = 0.0;   // 0 ~ 1
    particlesMass                = 1.0;   // 0 ~ 2
    particlesMassRnd             = 0.0;   // 0 ~ 1
    particlesSize                = 1.0;   // 0 ~ 10
    particlesSizeRnd             = 0.0;   // 0 ~ 1
    
    // Marker drawing forces
    markerRed                    = 0.28;
    markerGreen                  = 0.59;
    markerBlue                   = 1.0;
    markerOpacity                = 0.57;
    markerColor                  = ofFloatColor(markerRed, markerGreen, markerBlue);
    markerRandomColor            = false;
    
    // Input
    markersInput                 = false; // Fluid input are the IR markers?
    contourInput                 = false; // Fluid input is the depth contour?
    
    // Output
    drawVelocity                 = false;
    drawVelocityScalar           = false;
    drawPressure                 = false;
    drawVorticity                = false;
    drawTemperature              = false;
    drawMarkerFluid              = false;
    drawContourFluid             = false;
}

void Fluid::setup(int width, int height, float scaleFactor, bool doFasterInternalFormat){
    this->width = width;
    this->height = height;
    
    this->scaleFactor = scaleFactor;
    
    this->flowWidth = width/scaleFactor;
    this->flowHeight = height/scaleFactor;
    
    // Fluid
    fluid.setup(flowWidth, flowHeight, width, height);
    
    // Particles
    fluidParticles.setup(flowWidth, flowHeight, width, height);
    
    // Create rectangle with flow size
    rescaledRect.set(0, 0, flowWidth, flowHeight);
    
    // Setup marker drawing temporal Forces with default values
    numMarkerForces = 3;
    markerForces = new ftDrawForce[numMarkerForces];
    markerForces[0].setup(width, height, FT_DENSITY, true);
    markerForces[0].setName("draw full res");
    markerForces[0].setForce(ofVec4f(markerRed, markerGreen, markerBlue, markerOpacity));
    markerForces[0].setRadius(0.0140983);
    markerForces[0].setStrength(2.42295);

    markerForces[1].setup(flowWidth, flowHeight, FT_VELOCITY, true);
    markerForces[1].setName("draw flow res 1");
    markerForces[1].setRadius(0.0146189);
    markerForces[1].setStrength(0.96523);
   
    markerForces[2].setup(flowWidth, flowHeight, FT_TEMPERATURE, true);
    markerForces[2].setName("draw flow res 2");
    markerForces[2].setForce(ofVec4f(1, 0, 0, 1));
    markerForces[2].setRadius(0.02451);
    
    // Initialize marker drawing forces
    markerForceForces.resize(numMarkerForces);
    markerForceStrengths.resize(numMarkerForces);
    markerForceRadiuses.resize(numMarkerForces);
    markerForceEdges.resize(numMarkerForces);
    
    for (int i = 0; i < numMarkerForces; i++){
        markerForceForces[i] = markerForces[i].getForce();
        markerForceStrengths[i] = markerForces[i].getStrength();
        markerForceRadiuses[i] = markerForces[i].getRadius();
        markerForceEdges[i] = markerForces[i].getEdge();
    }
    
    // Allocate visualisation classes
    displayScalar.setup(flowWidth, flowHeight);
    densityDisplayScalar.setup(width, height);
    velocityField.setup(flowWidth/4, flowHeight/4);
    
    // Allocate fluid texture
//    fluidFlow.allocate(flowWidth, flowHeight, GL_RGBA32F);
    
    // allocate fluid FBO
    fluidFbo.allocate(flowWidth, flowWidth, GL_RGBA32F);
    
    // Allocate fluid pixels
    fluidVelocities.allocate(flowWidth, flowHeight, 4);
}

void Fluid::update(float dt, vector<irMarker> &markers, Contour &contour, float mouseX, float mouseY){
    
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
        
        if(contourInput){
            fluid.addVelocity(contour.getOpticalFlowDecay());
            fluid.addDensity(contour.getColorMask());
            fluid.addTemperature(contour.getLuminanceMask());
        }
        
        if(markersInput){
            // apply marker velocity and position forces
            for(unsigned int markerIdx = 0; markerIdx < markers.size(); markerIdx++){
                if (!markers[markerIdx].hasDisappeared){
                    for (int i=0; i<numMarkerForces; i++) {
                        if (markerForces[i].getType() == FT_VELOCITY){
                            ofVec2f velNormalized = ofVec2f(markers[markerIdx].velocity.x / width, markers[markerIdx].velocity.y / height);
                            markerForces[i].setForce(velNormalized);
                        }
                        ofVec2f posNormalized = ofVec2f(markers[markerIdx].smoothPos.x / width, markers[markerIdx].smoothPos.y / height);
                        markerForces[i].applyForce(posNormalized);
                    }
                }
            }
            
            if(markerRandomColor){
//                color.setHsb((ofGetFrameNum() % 255), 255, 255);
                markerColor.setHsb(ofMap(ofNoise(ofGetElapsedTimef()*0.3), 0.1, 0.9, 0.0, 1.0, true), 1.0, 1.0);
                markerRed   = markerColor.r;
                markerGreen = markerColor.g;
                markerBlue  = markerColor.b;
            }
            markerColor.set(markerRed, markerGreen, markerBlue, markerOpacity);
            
            updateDrawForces(dt);
        }

        fluid.update(dt);
        
        // Get fluid texture and save to pixels so we can operate with them
        fluidFbo.begin();
        ofPushStyle();
        ofClear(255, 255, 255, 0); // clear buffer
        fluid.getVelocity().draw(0, 0, flowWidth, flowHeight);
        ofPopStyle();
        fluidFbo.end();
        
        fluidFbo.readToPixels(fluidVelocities);
        
        if(particlesActive){
            // set fluid particles parameters
            fluidParticles.setSpeed(fluid.getSpeed());
            fluidParticles.setCellSize(fluid.getCellSize());
            fluidParticles.setBirthChance(particlesBirthChance);
            fluidParticles.setBirthVelocityChance(particlesBirthVelocityChance);
            fluidParticles.setLifeSpan(particlesLifetime);
            fluidParticles.setLifeSpanSpread(particlesLifetimeRnd);
            fluidParticles.setMass(particlesMass);
            fluidParticles.setMassSpread(particlesMassRnd);
            fluidParticles.setSize(particlesSize);
            fluidParticles.setSizeSpread(particlesSizeRnd);
            
            if(contourInput) fluidParticles.addFlowVelocity(contour.getOpticalFlowDecay());
            fluidParticles.addFluidVelocity(fluid.getVelocity());
        }
        fluidParticles.update(dt);
    }
    else if(activeStarted){
        activeStarted = false;
        isFadingIn = false;
        isFadingOut = true;
        startFadeIn = false;
        startFadeOut = true;
    }
}

void Fluid::draw(){
    if(isActive || isFadingOut){
        if(drawVelocity){
            ofPushStyle();
            ofEnableBlendMode(OF_BLENDMODE_ADD);
            velocityField.setVelocity(fluid.getVelocity());
            velocityField.draw(0, 0, width, height);
            ofPopStyle();
        }
        if(drawVelocityScalar){
            ofPushStyle();
            ofEnableBlendMode(OF_BLENDMODE_DISABLED);
            displayScalar.setSource(fluid.getVelocity());
            displayScalar.draw(0, 0, width, height);
            ofPopStyle();
        }
        if(drawPressure){
            ofPushStyle();
            ofEnableBlendMode(OF_BLENDMODE_DISABLED);
            displayScalar.setSource(fluid.getPressure());
            displayScalar.draw(0, 0, width, height);
            ofPopStyle();
        }
        if(drawTemperature){
            ofPushStyle();
            ofEnableBlendMode(OF_BLENDMODE_DISABLED);
            displayScalar.setSource(fluid.getTemperature());
            displayScalar.draw(0, 0, width, height);
            ofPopStyle();
        }
        if(drawVorticity){
            ofPushStyle();
            ofEnableBlendMode(OF_BLENDMODE_DISABLED);
            displayScalar.setSource(fluid.getConfinement());
            displayScalar.draw(0, 0, width, height);
            ofPopStyle();
        }
        if(drawMarkerFluid || drawContourFluid){
            ofPushStyle();
            ofEnableBlendMode(OF_BLENDMODE_ADD);
            ofSetColor(red, green, blue, opacity);
            fluid.draw(0, 0, width, height);
            ofPopStyle();
        }
        if(particlesActive){
            ofPushStyle();
            ofEnableBlendMode(OF_BLENDMODE_ADD);
            fluidParticles.draw(0, 0, width, height);
            ofPopStyle();
        }
    }
}

void Fluid::updateDrawForces(float dt){
    
    // Update marker drawing forces
    for (int i = 0; i < numMarkerForces; i++){
        if(i == 0) markerForces[i].setForce(markerColor);
        else markerForces[i].setForce(markerForceForces[i]);
        markerForces[i].setStrength(markerForceStrengths[i]);
        markerForces[i].setRadius(markerForceRadiuses[i]);
        markerForces[i].setEdge(markerForceEdges[i]);
    }
    
    for (int i = 0; i < numMarkerForces; i++){
        markerForces[i].update();
        if(markerForces[i].didChange()){
            // if a force is constant multiply by deltaTime
            float strength = markerForces[i].getStrength();
            if(!markerForces[i].getIsTemporary()){
                strength *= dt;
            }
            switch (markerForces[i].getType()){
                case FT_DENSITY:
                    fluid.addDensity(markerForces[i].getTexture(), strength);
                    break;
                case FT_VELOCITY:
                    fluid.addVelocity(markerForces[i].getTexture(), strength);
                    fluidParticles.addFlowVelocity(markerForces[i].getTexture(), strength);
                    break;
                case FT_TEMPERATURE:
                    fluid.addTemperature(markerForces[i].getTexture(), strength);
                    break;
                case FT_PRESSURE:
                    fluid.addPressure(markerForces[i].getTexture(), strength);
                    break;
                case FT_OBSTACLE:
                    fluid.addTempObstacle(markerForces[i].getTexture());
                default:
                    break;
            }
        }
    }
}

ofVec2f Fluid::getFluidOffset(ofPoint p){
    ofPoint p_ = p/scaleFactor;
    ofVec2f offset(0,0);
    
    if(rescaledRect.inside(p_)){
        offset.x = fluidVelocities[((int)p_.y*flowWidth+(int)p_.x)*4 + 0]; // r
        offset.y = fluidVelocities[((int)p_.y*flowWidth+(int)p_.x)*4 + 1]; // g
    }
        
    return offset;
}

void Fluid::reset(){
    fluid.reset();
}

void Fluid::resetDrawForces(){
    // reset temporal drawing forces
    for (int i = 0; i < numMarkerForces; i++){
        markerForces[i].reset();
    }
}

void Fluid::fadeIn(float dt){
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

void Fluid::fadeOut(float dt){
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