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

#include "ParticleSystem.h"

bool comparisonFunction(Particle * a, Particle * b) {
    return a->pos.x < b->pos.x;
}

ParticleSystem::ParticleSystem(){
    isActive            = false;        // Particle system is active?
    activeStarted       = false;        // Active has started?
    
    // Fading in/out
    isFadingIn          = false;        // Opacity fading in?
    isFadingOut         = false;        // Opacity fading out?
    startFadeIn         = false;        // Fade in has started?
    startFadeOut        = false;        // Fade out has started?
    elapsedFadeTime     = 0.0;          // Elapsed time of fade
    fadeTime            = 1.2;          // Transition time of fade
    
    // Opacity
    opacity             = 0.0;          // Actual opacity of the particles
    maxOpacity          = 255.0;        // Maximum opacity of particles

    // General properties
    immortal            = false;        // Are particles immortal?
    velocity            = 0.0;          // Initial velocity magnitude of newborn particles
    radius              = 3.0;          // Radius of the particles
    lifetime            = 5.0;          // Lifetime of particles
    red                 = 255.0;
    green               = 255.0;
    blue                = 255.0;
    
    // Specific properties
    nParticles          = 300;          // Number of particles born from the beginning
    bornRate            = 5.0;          // Number of particles born per frame

    // Emitter
    emitterSize         = 3.0;          // Size of the emitter area
    velocityRnd         = 20.0;         // Magnitude randomness % of the initial velocity
    velocityMotion      = 50.0;         // Marker motion contribution to the initial velocity
    lifetimeRnd         = 20.0;         // Randomness of lifetime
    radiusRnd           = 50.0;         // Randomness of radius

    // Grid
    gridRes             = 10;           // Resolution of the grid

    // Flocking
    lowThresh           = 0.1333;       // If dist. ratio lower than lowThresh separate
    highThresh          = 0.6867;       // Else if dist. ratio higher than highThresh attract. Else just align
    separationStrength  = 0.01f;        // Separation force
    attractionStrength  = 0.004f;       // Attraction force
    alignmentStrength   = 0.01f;        // Alignment force
    maxSpeed            = 80.0;         // Max speed particles
    flockingRadius      = 60.0;         // Radius of flocking

    // Graphic output
    sizeAge             = false;        // Decrease size when particles get older?
    opacityAge          = false;        // Decrease opacity when particles get older?
    colorAge            = false;        // Change color when particles get older?
    flickersAge         = false;        // Particle flickers opacity when about to die?
    isEmpty             = false;        // Draw only contours of the particles?
    drawLine            = false;        // Draw a line instead of a circle for the particle?
    drawStroke          = false;        // Draw stroke line around particle?
    strokeWidth         = 1.2;          // Stroke line width
    drawConnections     = false;        // Draw a connecting line between close particles?
    connectDist         = 15.0;         // Maximum distance to connect particles with line
    connectWidth        = 1.0;          // Connected line width

    // Physics
    friction            = 5.0;          // Friction to velocity 0~100
    gravity             = ofPoint(0, 0);// Makes particles react to gravity
    turbulence          = 0.0f;         // Turbulence perlin noise
    repulse             = false;        // Repulse particles between each other?
    bounce              = false;        // Bounce particles with the walls of the window?
    steer               = false;        // Steers direction before touching the walls of the window?
    infiniteWalls       = false;        // Infinite walls?
    bounceDamping       = true;         // Decrease velocity when particle bounces walls?
    repulseDist         = 5.0*radius;   // Repulse particle-particle distance
    
    returnToOriginForce = 10.0;         // How fast particle returns to its position

    // Behavior
    emit                = false;        // Born new particles in each frame?
    interact            = false;        // Can we interact with the particles?
    flock               = false;        // Particles have flocking behavior?
    flowInteraction     = false;        // Interact with particles using optical flow?
    fluidInteraction    = false;        // Interact with the fluid velocities?
    repulseInteraction  = false;        // Repulse particles from input?
    attractInteraction  = false;        // Attract particles to input?
    seekInteraction     = false;        // Make particles seek target (markers)?
    gravityInteraction  = false;        // Apply gravity force to particles touched with input?
    bounceInteraction   = false;        // Bounce particles with depth contour?
    returnToOrigin      = false;        // Make particles return to their born position?

    // Input
    markersInput        = false;        // Input are the IR markers?
    contourInput        = false;        // Input is the depth contour?
    interactionForce    = 100.0;        // Force of interaction
    interactionRadius   = 80.0;         // Radius of interaction
    
    emitAllTimeInside   = true;         // Emit particles every frame inside all the defined area?
    emitAllTimeContour  = false;        // Emit particles every frame only on the contour of the defined area?
    emitInMovement      = false;        // Emit particles only in regions that there has been some movement?

    useFlow             = true;         // Use optical flow to get the motion velocity?
    useFlowRegion       = false;        // Use optical flow region to get the motion velocity?
    useContourArea      = false;        // Use contour area to interact with particles?
    useContourVel       = false;        // Use contour velocities to interact with particles?
}

ParticleSystem::~ParticleSystem(){
    // Delete all the particles
    for (int i = 0; i < particles.size(); i++) {
        delete particles.at(i);
        particles.at(i) = NULL;
    }
    particles.clear();
}


void ParticleSystem::setup(ParticleMode particleMode, int width , int height){

    this->particleMode = particleMode;

    this->width = width;
    this->height = height;

    if(particleMode == EMITTER){
        emit = true;
        sizeAge             = true;        // Decrease size when particles get older?
        opacityAge          = true;        // Decrease opacity when particles get older?
        colorAge            = true;        // Change color when particles get older?
        velocity            = 20;
    }
    else if(particleMode == GRID){
        interact            = true;
        radiusRnd           = 0.0;
        returnToOrigin      = true;
        immortal            = true;
        velocity            = 0.0;
        velocityRnd         = 0.0;
        gravity             = ofPoint(0, 0);
        friction            = 100;
        createParticleGrid(width, height);
    }
    else if(particleMode == BOIDS){
        flock               = true;
//        interact            = true;
        bounceDamping       = false;
        immortal            = true;
        addParticles(nParticles);
    }
    else if(particleMode == ANIMATIONS){
        immortal            = false;
        
        friction            = 0.0;
        turbulence          = 0.0;
        bounce              = false;
        opacityAge          = true;
        sizeAge             = false;
        flickersAge         = false;
        
        if(animation == RAIN){
            radius          = 0.65;
            radiusRnd       = 10.0;
            
            velocity        = 80.0;
            velocityRnd     = 20.0;
            
            lifetime        = 1.3;
            lifetimeRnd     = 50.0;
            
            gravity         = ofPoint(0, 80);
            bounce          = true;
        }
        else if(animation == SNOW){
            radius          = 2.0;
            radiusRnd       = 30.0;
            
            velocity        = 40.0;
            velocityRnd     = 20.0;
            
            lifetime        = 2.8;
            lifetimeRnd     = 30.0;
            
            gravity         = ofPoint(0, 15);
            bounce          = true;
        }
        else if(animation == EXPLOSION){
            radius          = 7.0;
            radiusRnd       = 80.0;
            
            velocity        = 700.0;
            velocityRnd     = 70.0;
            
            lifetime        = 1.6;
            lifetimeRnd     = 40.0;
            
            gravity         = ofPoint(0, 100);
            friction        = 90.0;
            turbulence      = 15.0;
            flickersAge     = true;
            sizeAge         = true;
            opacityAge      = false;
            addParticles(nParticles);
        }
    }
}

void ParticleSystem::update(float dt, vector<irMarker>& markers, Contour& contour, Fluid& fluid){
    
    // if is active or we are fading out, update particles
    if(isActive || isFadingOut){
        // if it is the first frame where isActive is true and we are not fading out (hack to fix switching all time)
        // start fadeIn and change activeStarted to true so we dont enter anymore
        if(!activeStarted && !isFadingOut){
            activeStarted = true;
            isFadingIn = true;
            isFadingOut = false;
            startFadeIn = true;
            startFadeOut = false;
            opacity = 0.0;
            bornParticles();
        }
        
        if(isFadingIn) fadeIn(dt);
        else if(isFadingOut && !isActive) fadeOut(dt); // if it is not active and it is fading out, fade out
        else opacity = maxOpacity;
        
        // sort particles so it is more effective to do particle/particle interactions
        sort(particles.begin(), particles.end(), comparisonFunction);
        
        // compute radius squareds so we just do it once
        float interactionRadiusSqrd = interactionRadius*interactionRadius;
        float flockingRadiusSqrd = flockingRadius * flockingRadius;

        // ---------- (1) Delete inactive particles
        int i = 0;
        while (i < particles.size()){
            if (!particles[i]->isAlive){
                delete particles.at(i);
                particles.erase(particles.begin() + i);
                numParticles--;
            }
            else{
                i++;
            }
        }

        // ---------- (2) Calculate specific particle system behavior
        for(int i = 0; i < particles.size(); i++){
            if(interact){ // Interact particles with input
                if(markersInput){
                    irMarker* closestMarker;
                    if(particleMode == BOIDS) // get closest marker to particle
                        closestMarker = getClosestMarker(*particles[i], markers);
                    else // get closest marker to particle only if particle is inside interaction area
                        closestMarker = getClosestMarker(*particles[i], markers, interactionRadiusSqrd);

                    // Get direction vector to closest marker
                    // dir = closestMarker->smoothPos - particles[i]->pos;
                    // dir.normalize();

                    if(closestMarker != NULL){
                        if(flowInteraction){
                            float markerDistSqrd = particles[i]->pos.squareDistance(closestMarker->smoothPos);
                            float pct = 1 - (markerDistSqrd / interactionRadiusSqrd); // stronger on the inside
                            particles[i]->addForce(closestMarker->velocity*pct*interactionForce);
                        }
                        else if(repulseInteraction) particles[i]->addRepulsionForce(closestMarker->smoothPos, interactionRadiusSqrd, interactionForce);
                        else if(attractInteraction) particles[i]->addAttractionForce(closestMarker->smoothPos, interactionRadiusSqrd, interactionForce);
                        else if(seekInteraction){
                            particles[i]->seek(closestMarker->smoothPos, interactionRadiusSqrd, interactionForce*10.0);
                        }
                        else if(gravityInteraction){
                            particles[i]->addForce(ofPoint(ofRandom(-100, 100), 500.0)*particles[i]->mass);
                            particles[i]->isTouched = true;
                        }
                        else if(bounceInteraction){
                            unsigned int contourIdx = -1;
                            ofPoint closestPointInContour = getClosestPointInContour(*particles[i], contour, true, &contourIdx);
                            if(closestPointInContour != ofPoint(-1, -1)){
                                if(contourIdx != -1) particles[i]->contourBounce(contour.contours[contourIdx]);
                            }
                        }
                    }
                    else if(gravityInteraction && particles[i]->isTouched){
                        particles[i]->addForce(ofPoint(0, 500.0)*particles[i]->mass);
                    }
                }
                if(contourInput){
                    unsigned int contourIdx = -1;
                    ofPoint closestPointInContour;
                    if(particleMode == BOIDS && seekInteraction) // get closest point to particle
                        closestPointInContour = getClosestPointInContour(*particles[i], contour, false, &contourIdx);
                    else // get closest point to particle only if particle is inside contour
                        closestPointInContour = getClosestPointInContour(*particles[i], contour, true, &contourIdx);
                    
                    if(flowInteraction){
                        ofPoint frc = contour.getFlowOffset(particles[i]->pos);
                        particles[i]->addForce(frc*interactionForce);
                    }

                    if(closestPointInContour != ofPoint(-1, -1)){
                        if(repulseInteraction){ // it is an attractForce but result is more logical saying repulse
                            particles[i]->addAttractionForce(closestPointInContour, interactionRadiusSqrd, interactionForce);
                        }
                        else if(attractInteraction){
                            particles[i]->addRepulsionForce(closestPointInContour, interactionRadiusSqrd, interactionForce);
                        }
                        else if(seekInteraction){
                            particles[i]->seek(closestPointInContour, interactionRadiusSqrd, interactionForce*10.0);
                        }
                        else if(gravityInteraction){
                            particles[i]->addForce(ofPoint(ofRandom(-100, 100), 500.0)*particles[i]->mass);
                            particles[i]->isTouched = true;
                        }
                        else if(bounceInteraction){
                            if(contourIdx != -1) particles[i]->contourBounce(contour.contours[contourIdx]);
                        }
                    }
                    else if(gravityInteraction && particles[i]->isTouched){
                        particles[i]->addForce(ofPoint(0.0, 500.0)*particles[i]->mass);
                    }
                }
                if(fluidInteraction){
                    ofPoint frc = fluid.getFluidOffset(particles[i]->pos);
                    particles[i]->addForce(frc*interactionForce);
                }
            }

            if(flock){ // Flocking behavior
                particles[i]->flockingRadiusSqrd    =   flockingRadiusSqrd;

                particles[i]->separationStrength    =   separationStrength;
                particles[i]->alignmentStrength     =   alignmentStrength;
                particles[i]->attractionStrength    =   attractionStrength;

                particles[i]->lowThresh             =   lowThresh;
                particles[i]->highThresh            =   highThresh;
                particles[i]->maxSpeed              =   maxSpeed;
            }

            if(returnToOrigin && particleMode == GRID && !gravityInteraction) particles[i]->returnToOrigin(100, returnToOriginForce);

            if(particleMode == ANIMATIONS && animation == SNOW){
                float windX = ofSignedNoise(particles[i]->pos.x * 0.003, particles[i]->pos.y * 0.006, ofGetElapsedTimef() * 0.1) * 3.0;
                ofPoint frc;
                frc.x = windX + ofSignedNoise(particles[i]->id, particles[i]->pos.y * 0.04) * 8.0;
                frc.y = ofSignedNoise(particles[i]->id, particles[i]->pos.x * 0.006, ofGetElapsedTimef() * 0.2) * 3.0;
                particles[i]->addForce(frc*particles[i]->mass);
            }
        }

        if(emit){ // Born new particles
            if(markersInput){
                for(unsigned int i = 0; i < markers.size(); i++){
                    if (!markers[i].hasDisappeared){
                        if(emitInMovement){
                            float n = ofMap(markers[i].velocity.lengthSquared(), 0.5, 300.0, 0.0, bornRate, true);
                            addParticles(n, markers[i]);
                        }
                        else{
                            float range = ofMap(bornRate, 0, 150, 0, 20);
                            if(bornRate > 0.1) addParticles(ofRandom(bornRate-range, bornRate+range), markers[i]);
                        }
                    }
                }
            }
            if(contourInput){
                if(emitInMovement){
                    for(unsigned int i = 0; i < contour.vMaskContours.size(); i++){
                        // born more particles if bigger area
                        float bornNum = bornRate * abs(contour.vMaskContours[i].getArea())/1500.0;
                        addParticles(bornNum, contour.vMaskContours[i], contour);
                    }
                }
                else{
                    for(unsigned int i = 0; i < contour.contours.size(); i++){
                        float range = ofMap(bornRate, 0, 150, 0, 30);
                        addParticles(ofRandom(bornRate-range, bornRate+range), contour.contours[i], contour);
                    }
                }
            }
        }

        // Keep adding particles if it is an animation (unless it is an explosion)
        if(particleMode == ANIMATIONS && animation != EXPLOSION){
            float range = ofMap(bornRate, 0, 60, 0, 15);
            addParticles(ofRandom(bornRate-range, bornRate+range));
        }

        if(flock) flockParticles();
        if(repulse) repulseParticles();

        // ---------- (3) Add some general behavior and update the particles
        for(int i = 0; i < particles.size(); i++){
            particles[i]->addForce(gravity*particles[i]->mass);
            particles[i]->addNoise(turbulence);
            
            particles[i]->opacity           = opacity;
            particles[i]->friction          = 1-friction/1000;
            particles[i]->bounces           = bounce;
            particles[i]->bounceDamping     = bounceDamping;
            particles[i]->steers            = steer;
            particles[i]->infiniteWalls     = infiniteWalls;
            
            // update attributes also from immortal particle systems like GRID and BOIDS
            if(immortal){
                if(particleMode != BOIDS)
                    particles[i]->radius    = radius;
                particles[i]->color         = ofColor(red, green, blue);
                particles[i]->isEmpty       = isEmpty;
                particles[i]->drawLine      = drawLine;
                particles[i]->drawStroke    = drawStroke;
                particles[i]->strokeWidth   = strokeWidth;
            }
            
            if(gravityInteraction) particles[i]->bounces = true;

            particles[i]->update(dt);
        }
    }
    else if(activeStarted){
        activeStarted = false;
        isFadingIn = false;
        isFadingOut = true;
        startFadeIn = false;
        startFadeOut = true;
        killParticles();
    }
}

void ParticleSystem::draw(){
    if(isActive || isFadingOut){
        ofPushStyle();
        //Draw lines between near points
        if(drawConnections){
            ofPushStyle();
            float connectDistSqrd = connectDist*connectDist;
            ofSetColor(ofColor(red, green, blue), opacity);
            ofSetLineWidth(connectWidth);
            for(int i = 0; i < particles.size(); i++){
                for(int j = i-1; j >= 0; j--){
//                    if (fabs(particles[i]->pos.x - particles[j]->pos.x) > connectDist) break; // to speed the loop
                    if(particles[i]->pos.squareDistance(particles[j]->pos) < connectDistSqrd){
                        ofDrawLine(particles[i]->pos, particles[j]->pos);
                    }
                }
            }
            ofPopStyle();
        }
        // Draw particles
        for(int i = 0; i < particles.size(); i++){
            particles[i]->draw();
        }
        ofPopStyle();
    }
}

void ParticleSystem::addParticle(ofPoint pos, ofPoint vel, ofColor color, float radius, float lifetime){
    Particle * newParticle = new Particle();
    float id = totalParticlesCreated;

    newParticle->sizeAge        = sizeAge;
    newParticle->opacityAge     = opacityAge;
    newParticle->flickersAge    = flickersAge;
    newParticle->colorAge       = colorAge;
    newParticle->isEmpty        = isEmpty;
    newParticle->drawLine       = drawLine;
    newParticle->drawStroke     = drawStroke;
    newParticle->strokeWidth    = strokeWidth;
    
    newParticle->friction       = 1 - friction/1000;

    newParticle->width = width;
    newParticle->height = height;
    
    if(particleMode == GRID){
        newParticle->immortal = true;
    }
    else if(particleMode == BOIDS){
        newParticle->limitSpeed = true;
        newParticle->immortal = true;
    }
    else if(particleMode == ANIMATIONS){
        if(animation == SNOW) newParticle->damping = 0.05;
        else newParticle->damping = 0.2;
        newParticle->bounceTop = false;
    }

    newParticle->setup(id, pos, vel, color, radius, lifetime);
    particles.push_back(newParticle);

    numParticles++;
    totalParticlesCreated++;
}

void ParticleSystem::addParticles(int n){
    for(int i = 0; i < n; i++){
        ofPoint pos = ofPoint(ofRandom(width), ofRandom(height));
        ofPoint vel = randomVector()*(velocity+randomRange(velocityRnd, velocity));

        if(particleMode == ANIMATIONS && (animation == RAIN || animation == SNOW)){
            pos = ofPoint(ofRandom(width), ofRandom(-5*radius, -10*radius));
            vel.x = 0;
            vel.y = velocity+randomRange(velocityRnd, velocity); // make particles all be going down when born
        }
        else if(particleMode == ANIMATIONS && animation == EXPLOSION){
            pos = ofPoint(ofRandom(width), ofRandom(height, height+radius*15));
            vel.x = 0;
            vel.y = -velocity-randomRange(velocityRnd, velocity); // make particles all be going up when born
        }

        float initialRadius = radius + randomRange(radiusRnd, radius);
        float lifetime = this->lifetime + randomRange(lifetimeRnd, this->lifetime);

        addParticle(pos, vel, ofColor(red, green, blue), initialRadius, lifetime);
    }
}

void ParticleSystem::addParticles(int n, const irMarker& marker){
    for(int i = 0; i < n; i++){
        ofPoint pos;
        if(emitAllTimeInside || emitInMovement){
            pos = marker.smoothPos + randomVector()*ofRandom(0, emitterSize);
        }
        else if(emitAllTimeContour){
            pos = marker.smoothPos + randomVector()*emitterSize;
        }
        ofPoint randomVel = randomVector()*(velocity+randomRange(velocityRnd, velocity));
        ofPoint motionVel = marker.velocity*(velocity/5.0+randomRange(velocityRnd, velocity/5.0)); // /5.0 to reduce effect
        ofPoint vel = randomVel*(velocityRnd/100.0) + motionVel*(velocityMotion/100.0);

        float initialRadius = radius + randomRange(radiusRnd, radius);
        float lifetime = this->lifetime + randomRange(lifetimeRnd, this->lifetime);

        addParticle(pos, vel, ofColor(red, green, blue), initialRadius, lifetime);
    }
}

void ParticleSystem::addParticles(int n, const ofPolyline& contour, Contour& flow){
    for(int i = 0; i < n; i++){

        ofPoint pos, randomVel, motionVel, vel;

        // Create random particles inside contour polyline
        if(emitAllTimeInside || emitInMovement){
            ofRectangle box = contour.getBoundingBox(); // so it is easier that the particles are born inside contour
            ofPoint center = box.getCenter();
            pos.x = center.x + (ofRandom(1.0f) - 0.5f) * box.getWidth();
            pos.y = center.y + (ofRandom(1.0f) - 0.5f) * box.getHeight();

            while(!contour.inside(pos)){
                pos.x = center.x + (ofRandom(1.0f) - 0.5f) * box.getWidth();
                pos.y = center.y + (ofRandom(1.0f) - 0.5f) * box.getHeight();
            }

            // set velocity to random vector direction with 'velocity' as magnitude
            randomVel = randomVector()*(velocity+randomRange(velocityRnd, velocity));
        }

        // Create particles only on the contour polyline
        else if(emitAllTimeContour){
            float indexInterpolated = ofRandom(0, contour.size());
            pos = contour.getPointAtIndexInterpolated(indexInterpolated);

            // Use normal vector in surface as vel. direction so particle moves out of the contour
            randomVel = -contour.getNormalAtIndexInterpolated(indexInterpolated)*(velocity+randomRange(velocityRnd, velocity));
        }

        if(true){ // get velocity vector in particle pos
            motionVel = flow.getFlowOffset(pos)*(velocity*5.0+randomRange(velocityRnd, velocity*5.0));
        }
        else if(useContourVel){ // slower and poorer result
            motionVel = flow.getVelocityInPoint(pos)*(velocity*5.0+randomRange(velocityRnd, velocity*5.0));
        }
        vel = randomVel*(velocityRnd/100.0) + motionVel*(velocityMotion/100.0);
        pos += randomVector()*emitterSize; // randomize position within a range of emitter size

        float initialRadius = radius + randomRange(radiusRnd, radius);
        float lifetime = this->lifetime + randomRange(lifetimeRnd, this->lifetime);

        addParticle(pos, vel, ofColor(red, green, blue), initialRadius, lifetime);
    }
}

void ParticleSystem::createParticleGrid(int width, int height){
    for(int y = 0; y < height/gridRes; y++){
        for(int x = 0; x < width/gridRes; x++){
            int xi = (x + 0.5f) * gridRes;
            int yi = (y + 0.5f) * gridRes;
//            float initialRadius = (float)gridRes / 2.0f;
//            float initialRadius = ofRandom(1.0f, 6.0f);
//            float initialRadius = cos(yi * 0.1f) + sin(xi * 0.1f) + 2.0f;
//            float initialRadius = (sin(yi * xi) + 1.0f) * 2.0f;
//            float xyOffset = sin( cos( sin( yi * 0.3183f ) + cos( xi * 0.3183f ) ) ) + 1.0f;
//            float initialRadius = xyOffset * xyOffset * 1.8f;
            ofPoint vel = ofPoint(0, 0);
            addParticle(ofPoint(xi, yi), vel, ofColor(red, green, blue), radius, lifetime);
        }
    }
}

void ParticleSystem::removeParticles(int n){
    n = MIN(particles.size(), n);
    for(int i = 0; i < n; i++){
        particles[i]->immortal = false;
    }
}

void ParticleSystem::killParticles(){
    for(int i = 0; i < particles.size(); i++){
        particles[i]->immortal = false;
    }
}

void ParticleSystem::resetTouchedParticles(){
    for(int i = 0; i < particles.size(); i++){
        particles[i]->isTouched = false;
    }
}

void ParticleSystem::bornParticles(){
    // Kill all the remaining particles before creating new ones
    if(!particles.empty()){
        for(int i = 0; i < particles.size(); i++){
            particles[i]->isAlive = false;
        }
    }
    
    setup(particleMode, width, height); // resets the settings to default

//    if(particleMode == GRID){
//        createParticleGrid(width, height);
//    }
//    else if(particleMode == RANDOM || particleMode == BOIDS || particleMode == ANIMATIONS){
//        addParticles(nParticles);
//    }
}

void ParticleSystem::setAnimation(Animation animation){
    this->animation = animation;
}

void ParticleSystem::repulseParticles(){
    float repulseDistSqrd = repulseDist*repulseDist;
    for(int i = 1; i < particles.size(); i++){
        for(int j = i-1; j >= 0; j--){
            if (fabs(particles[i]->pos.x - particles[j]->pos.x) > repulseDist) break; // to speed the loop
            particles[i]->addRepulsionForce( *particles[j], repulseDistSqrd, 8.0);
//            particles[i]->addRepulsionForce( *particles[j], 30.0);
        }
    }
}

void ParticleSystem::flockParticles(){
    for(int i = 0; i < particles.size(); i++){
        for(int j = i-1; j >= 0; j--){
            if (fabs(particles[i]->pos.x - particles[j]->pos.x) > flockingRadius) break;
            particles[i]->addFlockingForces(*particles[j]);
        }
    }
}

ofPoint ParticleSystem::randomVector(){
    float angle = ofRandom((float)M_PI * 2.0f);
    return ofPoint(cos(angle), sin(angle));
}

float ParticleSystem::randomRange(float percentage, float value){
    return ofRandom(value-(percentage/100)*value, value+(percentage/100)*value);
}

irMarker* ParticleSystem::getClosestMarker(const Particle &particle, vector<irMarker> &markers, float interactionRadiusSqrd){
    irMarker* closestMarker = NULL;
    float minDistSqrd = interactionRadiusSqrd;
    
    // Get closest marker to particle
    for(int markerIndex = 0; markerIndex < markers.size(); markerIndex++){
        if (!markers[markerIndex].hasDisappeared){
            float markerDistSqrd = particle.pos.squareDistance(markers[markerIndex].smoothPos);
            if(markerDistSqrd < minDistSqrd){
                minDistSqrd = markerDistSqrd;
                closestMarker = &markers[markerIndex];
            }
        }
    }
    return closestMarker;
}

// Closest marker without distance limit
irMarker* ParticleSystem::getClosestMarker(const Particle &particle, vector<irMarker> &markers){
    irMarker* closestMarker = NULL;
    float minDistSqrd = 999999999;
    
    // Get closest marker to particle
    for(int markerIndex = 0; markerIndex < markers.size(); markerIndex++){
        if (!markers[markerIndex].hasDisappeared){
            float markerDistSqrd = particle.pos.squareDistance(markers[markerIndex].smoothPos);
            if(markerDistSqrd < minDistSqrd){
                minDistSqrd = markerDistSqrd;
                closestMarker = &markers[markerIndex];
            }
        }
    }
    return closestMarker;
}

ofPoint ParticleSystem::getClosestPointInContour(const Particle& particle, const Contour& contour, bool onlyInside, unsigned int* contourIdx){
    ofPoint closestPoint(-1, -1);
    float minDistSqrd = 999999999;

    // Get closest point to particle from the different contours
    for(unsigned int i = 0; i < contour.contours.size(); i++){
        if(!onlyInside || contour.contours[i].inside(particle.pos)){
            ofPoint candidatePoint = contour.contours[i].getClosestPoint(particle.pos);
            float pointDistSqrd = particle.pos.squareDistance(candidatePoint);
            if(pointDistSqrd < minDistSqrd){
                minDistSqrd = pointDistSqrd;
                closestPoint = candidatePoint;
                if(contourIdx != NULL) *contourIdx = i; // save contour index
            }
        }
    }
    return closestPoint;
}

void ParticleSystem::fadeIn(float dt){
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

void ParticleSystem::fadeOut(float dt){
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
