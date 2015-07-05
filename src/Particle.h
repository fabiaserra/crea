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

#pragma once
#include "ofMain.h"
#include "irMarker.h"

class Particle
{
    public:
        Particle();

        void setup(float id, ofPoint pos, ofPoint vel, ofColor color, float initialRadius, float lifetime);
        void update(float dt);
        void draw();

        void addForce(ofPoint force);
        void addNoise(float turbulence);
        void addRepulsionForce(ofPoint posOfForce, float radiusSqrd, float scale);
        void addAttractionForce(ofPoint posOfForce, float radiusSqrd, float scale);
        void addRepulsionForce(Particle &p, float radiusSqrd, float scale);
        void addAttractionForce(Particle &p, float radiusSqrd, float scale);
        void addRepulsionForce(Particle &p, float scale);
        void returnToOrigin(float radiusSqrd, float scale);

        void addFlockingForces(Particle &p);
        void seek(ofPoint target, float radiusSqrd, float scale);
        void seek(ofPoint target, float scale);
        void pullToCenter();
        void limitVelocity();

        void marginsBounce();
        void marginsSteer();
        void marginsWrap();
    
        void contourBounce(ofPolyline contour);

        void kill();
//--------------------------------------------------------------
        float opacity;
//--------------------------------------------------------------
        ofPoint pos;            // Position
        ofPoint prevPos;        // Previous position
        ofPoint iniPos;         // Initial position
        ofPoint vel;            // Velocity
        ofPoint acc;            // Acceleration
        ofPoint frc;            // Force
        ofColor color;          // Color
// --------------------------------------------------------------
        ofPolyline contour;     // Particle creation boundaries
// --------------------------------------------------------------
        float id;               // Particle ID
        float age;              // Time of living
        float mass;             // Mass of the particle
        float lifetime;         // Allowed lifetime
        float friction;         // Decay of the velocity
        float initialRadius;    // Radius of the particle when borns
        float radius;           // Radius of the particle
        float originalHue;      // Initial hue color
// --------------------------------------------------------------
        bool immortal;          // Can the particle die?
        bool isAlive;           // Is the particle alive?
        bool isTouched;         // Particle has been activated through some event

        bool sizeAge;           // Particle changes size with age?
        bool opacityAge;        // Particle changes opacity with age?
        bool flickersAge;       // Particle flickers opacity when about to die?
        bool colorAge;          // Particle changes color with age?
        bool isEmpty;           // Draw only contour of the particle?
        bool drawLine;          // Draw particle as a line from prevPos to pos?
        bool drawStroke;        // Draw stroke line around particle?
        float strokeWidth;      // Stroke line width

        bool limitSpeed;        // Limit the speed of the particle?
        bool bounceDamping;     // Decrease velocity when particle bounces?

        bool bounces;           // Particle bounces with the window margins?
        bool bounceTop;         // Particle bounces with top margin?
        bool steers;            // Particle steers direction before touching the walls?
        bool infiniteWalls;     // Particle goes back to the opposite wall?
// --------------------------------------------------------------
        float flockingRadiusSqrd;
        float lowThresh;        // separate
        float highThresh;       // align
        float separationStrength;
        float alignmentStrength;
        float attractionStrength;
        float maxSpeed;         // Maximum speed
        float damping;          // Damping when particle bounces walls
// --------------------------------------------------------------
        int width;              // Particle boundaries
        int height;
};

