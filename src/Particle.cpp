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

#include "Particle.h"

Particle::Particle(){
    isAlive         = true;

    opacity         = 255;

    isTouched       = false;

    immortal        = false;

    bounces         = false;
    bounceTop       = true;
    steers          = false;
    infiniteWalls   = false;

    sizeAge         = false;
    opacityAge      = false;
    colorAge        = false;
    flickersAge     = false;
    isEmpty         = false;
    drawLine        = false;
    drawStroke      = false;
    strokeWidth     = 1.2;

    limitSpeed      = false;
    maxSpeed        = 120.0;
    bounceDamping   = true;
    damping         = 0.6;

    age             = 0;

    width           = ofGetWidth();
    height          = ofGetHeight();
}

void Particle::setup(float id, ofPoint pos, ofPoint vel, ofColor color, float initialRadius, float lifetime){
    this->id = id;
    this->pos = pos;
    this->vel = vel;
    this->color = color;
    this->initialRadius = initialRadius;
    this->lifetime = lifetime;

    this->radius = initialRadius;
    this->mass = initialRadius * initialRadius * 0.005f;
    this->prevPos = pos;
    this->iniPos = pos;
    this->originalHue = color.getHue();
}

void Particle::update(float dt){
    if(isAlive){
        // Update position
        acc = frc/mass;     // Newton's second law F = m*a
        vel += acc*dt;      // Euler's method
        vel *= friction;    // Decay velocity
        if(limitSpeed) limitVelocity();
        pos += vel*dt;
        frc.set(0, 0);      // Restart force

        // Update age and check if particle has to die
        age += dt;
        if(!immortal && age >= lifetime) isAlive = false;
        else if(immortal) age = fmodf(age, lifetime);

        // Decrease particle radius with age
        if (sizeAge) radius = initialRadius * (1.0f - (age/lifetime));

        // Decrease particle opacity with age
        if (opacityAge) opacity *= (1.0f - (age/lifetime));
        if (flickersAge){
//            opacity *= (1.0f - (age/lifetime)*0.4);
            if((age/lifetime) > 0.75 && ofRandomf() > (1.4 - age/lifetime))
                opacity *= 0.5;
        }

        // Change particle color with age
        if (colorAge){
            color.setBrightness(ofMap(age, 0, lifetime, 255, 180));
            color.setHue(ofMap(age, 0, lifetime, originalHue, originalHue-100));
        }

        // hackish way to make particles glitter when they slow down a lot
//        if(vel.lengthSquared() < 5.0) {
//            vel.x = ofRandom(-10, 10);
//            vel.y = ofRandom(-10, 10);
//        }

        // Bounce particle with the window margins
        if(bounces){
            marginsBounce();
        }
        else if(steers){
            marginsSteer();
        }
        else if(infiniteWalls){
            marginsWrap();
        }
    }
}

void Particle::draw(){
    if(isAlive){
        ofPushStyle();
        ofSetColor(color, opacity);

        if(isEmpty){
            ofNoFill();
            ofSetLineWidth(2);
        }
        else{
            ofFill();
        }

        if(!drawLine){
            int resolution = ofMap(fabs(radius), 0, 10, 6, 22, true);
            ofSetCircleResolution(resolution);
            ofDrawCircle(pos, radius);
            if(drawStroke){
                ofPushStyle();
                ofNoFill();
                ofSetLineWidth(strokeWidth);
                ofSetColor(0, opacity);
                ofDrawCircle(pos, radius);
                ofPopStyle();
            }
        }
        else{
//            if(pos.squareDistance(prevPos) >= 25) ofDrawLine(pos, pos-vel.getNormalized()*5);
//            else ofDrawLine(pos, prevPos);
//            prevPos = pos;
            ofSetLineWidth(ofMap(radius, 0, 15, 1, 5, true));
            ofDrawLine(pos, pos-vel.getNormalized()*radius);
        }

        ofPopStyle();
    }
}

void Particle::addForce(ofPoint force){
    frc += force;
}

void Particle::addNoise(float turbulence){
    // Perlin noise
    float angle = ofSignedNoise(id*0.001f, pos.x * 0.005f,  pos.y * 0.005f, ofGetElapsedTimef() * 0.1f) * 20.0f;
    ofPoint noiseVector(cos(angle), sin(angle));
    if(!immortal) frc += noiseVector * turbulence * age; // if immortal this doesn't affect, age == 0
    else frc += noiseVector * turbulence;
}

void Particle::addRepulsionForce(ofPoint posOfForce, float radiusSqrd, float scale){

    // (1) calculate the direction to force source and distance
    ofPoint dir         = pos - posOfForce;
    float distSqrd      = pos.squareDistance(posOfForce); // faster than length or distance (no square root)

    // (2) if close enough update force
    if (distSqrd < radiusSqrd){
        float pct = 1 - (distSqrd / radiusSqrd);  // stronger on the inside
        dir.normalize();
        frc += dir * scale * pct;
    }
}

void Particle::addRepulsionForce(Particle &p, float radiusSqrd, float scale){
    
    // (1) make a vector of where this particle p is:
    ofPoint posOfForce;
    posOfForce.set(p.pos.x, p.pos.y);
    
    // (2) calculate the direction to particle and distance
    ofPoint dir         = pos - posOfForce;
    float distSqrd      = pos.squareDistance(posOfForce); // faster than length or distance (no square root)
    
    // (3) if close enough update both forces
    if (distSqrd < radiusSqrd){
        float pct = 1 - (distSqrd / radiusSqrd);  // stronger on the inside
        dir.normalize();
        frc   += dir * scale * pct;
        p.frc -= dir * scale * pct;
    }
}

void Particle::addRepulsionForce(Particle &p, float scale){
    // (1) make radius of repulsion equal to particle's radius sum
    float radius        = this->radius + p.radius;
    float radiusSqrd    = radius*radius;
    // (2) call addRepulsion force with the computed radius
    addRepulsionForce(p, radiusSqrd, scale);
}

void Particle::addAttractionForce(ofPoint posOfForce, float radiusSqrd, float scale){

    // (1) calculate the direction to force source and distance
    ofPoint dir         = pos - posOfForce;
    float distSqrd      = pos.squareDistance(posOfForce); // faster than length or distance (no square root)

    // (2) if close enough update force
    if (distSqrd < radiusSqrd){
        float pct = 1 - (distSqrd / radiusSqrd);  // stronger on the inside
        dir.normalize();
        frc -= dir * scale * pct;
    }
}

void Particle::addAttractionForce(Particle &p, float radiusSqrd, float scale){
    
    // (1) make a vector of where this particle p is:
    ofPoint posOfForce;
    posOfForce.set(p.pos.x, p.pos.y);
    
    // (2) calculate the direction to particle and distance
    ofPoint dir         = pos - posOfForce;
    float distSqrd      = pos.squareDistance(posOfForce); // faster than length or distance (no square root)
    
    // (3) if close enough update both forces
    if (distSqrd < radiusSqrd){
        float pct = 1 - (distSqrd / radiusSqrd);  // stronger on the inside
        dir.normalize();
        frc   -= dir * scale * pct;
        p.frc += dir * scale * pct;
    }
}

//------------------------------------------------------------------
void Particle::returnToOrigin(float radiusSqrd, float scale){
    // (1) calculate the direction to origin position and distance
    ofPoint dirToOrigin = iniPos - pos;
    float distSqrd = pos.squareDistance(iniPos);
    
    // (2) set force depending on the distance
    float pct = 1;
    if(distSqrd < radiusSqrd){
        pct = distSqrd / radiusSqrd; // decrease force when closer to origin
    }
    
    // (3) update force
    dirToOrigin.normalize();
    frc += dirToOrigin * scale * pct;
}

void Particle::addFlockingForces(Particle &p){
    ofPoint dir = pos - p.pos;
    float distSqrd = pos.squareDistance(p.pos);

    if(0.01f < distSqrd && distSqrd < flockingRadiusSqrd){ // if neighbor particle within zone radius...

        float percent = distSqrd/flockingRadiusSqrd;

        // Separate
        if(percent < lowThresh){            // ... and is within the lower threshold limits, separate
            float F = (lowThresh/percent - 1.0f) * separationStrength;
            dir = dir.getNormalized() * F;
            frc += dir;
            p.frc -= dir;
        }
        // Align
        else if(percent < highThresh){      // ... else if it is within the higher threshold limits, align
            float threshDelta = highThresh - lowThresh;
            float adjustedPercent = (percent - lowThresh) / threshDelta;
            float F = (0.5f - cos(adjustedPercent * M_PI * 2.0f) * 0.5f + 0.5f) * alignmentStrength;
            frc += p.vel.getNormalized() * F;
            p.frc += vel.getNormalized() * F;
        }
        // Attract
        else{                               // ... else, attract
            float threshDelta = 1.0f - highThresh;
            float adjustedPercent = (percent - highThresh) / threshDelta;
            float F = (0.5f - cos(adjustedPercent * M_PI * 2.0f) * 0.5f + 0.5f) * attractionStrength;
            dir = dir.getNormalized() * F;
            frc -= dir;
            p.frc += dir;
        }
    }
}

void Particle::pullToCenter(){
    ofPoint center(width/2, height/2);
    ofPoint dirToCenter = pos - center;
    float distToCenterSqrd = dirToCenter.lengthSquared();
    float distThresh = 900.0f;

    if(distToCenterSqrd > distThresh){
        dirToCenter.normalize();
        float pullStrength = 0.000015f;
        frc -= dirToCenter * ( ( distToCenterSqrd - distThresh ) * pullStrength );
    }
}

void Particle::seek(ofPoint target, float radiusSqrd, float scale){
    // (1) calculate the direction to target & length
    ofPoint dirToTarget = target - pos;
    float distSqrd = pos.squareDistance(target);
    
    // (2) scale force depending on the distance
    float pct = 1;
    if(distSqrd < radiusSqrd){
        pct = distSqrd / radiusSqrd; // decrease force when closer to target
    }
    
    // (3) update force
    dirToTarget.normalize();
    frc += dirToTarget * scale * pct;
}

// seek target independent of distance
void Particle::seek(ofPoint target, float scale){
    // (1) calculate the direction to target & length
    ofPoint dirToTarget = target - pos;
    
    // (2) scale force randomly
    float pct = ofRandom(0, 1);
    
    // (3) update force
    dirToTarget.normalize();
    vel += dirToTarget * scale * pct;
}


void Particle::marginsBounce(){
    bool isBouncing = false;

    if(pos.x > width-radius){
        pos.x = width-radius;
        vel.x *= -1.0;
    }
    else if(pos.x < radius){
        pos.x = radius;
        vel.x *= -1.0;
    }
    if(pos.y > height-radius){
        pos.y = height-radius;
        vel.y *= -1.0;
        isBouncing = true;
    }
    else if(bounceTop && pos.y < radius){
        pos.y = radius;
        vel.y *= -1.0;
    }

    if (isBouncing && bounceDamping){
        vel *= damping;
    }
}

void Particle::marginsSteer(){
    float margin = radius*10;

    if(pos.x > width-margin){
        vel.x -= ofMap(pos.x, width-margin, width, maxSpeed/1000.0, maxSpeed/10.0);
    }
    else if(pos.x < margin){
        vel.x += ofMap(pos.x, 0, margin, maxSpeed/10.0, maxSpeed/1000.0);
    }

    if(pos.y > height-margin){
        vel.y -= ofMap(pos.y, height-margin, height, maxSpeed/1000.0, maxSpeed/10.0);
    }
    else if(pos.y < margin){
        vel.y += ofMap(pos.y, 0, margin, maxSpeed/10.0, maxSpeed/10.0);
    }
}

void Particle::marginsWrap(){
    if(pos.x-radius > (float)width){
        pos.x = -radius;
    }
    else if(pos.x+radius < 0.0){
        pos.x = width;
    }

    if(pos.y-radius > (float)height){
        pos.y = -radius;
    }
    else if(pos.y+radius < 0.0){
        pos.y = height;
    }
}

void Particle::contourBounce(ofPolyline contour){
    unsigned int index;
    ofPoint contactPoint = contour.getClosestPoint(pos, &index);
    ofVec2f normal = contour.getNormalAtIndex(index);
    vel = vel - 2*vel.dot(normal)*normal; //reflection vector
    vel *= 0.35; // damping
    age += 0.5;
}

void Particle::kill(){
    isAlive = false;
}

void Particle::limitVelocity(){
    if(vel.lengthSquared() > (maxSpeed*maxSpeed)){
        vel.normalize();
        vel *= maxSpeed;
    }
}

