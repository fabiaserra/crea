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
#include "Particle.h"
#include "irMarker.h"
#include "Contour.h"
#include "Fluid.h"

enum ParticleMode {EMITTER, BOIDS, GRID, RANDOM, ANIMATIONS};
enum InputSource {MARKERS, CONTOUR};
enum Animation {SNOW, RAIN, EXPLOSION};

class ParticleSystem
{
    public:
        ParticleSystem();
        ~ParticleSystem();

        void setup(ParticleMode particleMode, int width, int height);
        void update(float dt, vector<irMarker>& markers, Contour& contour, Fluid& fluid);
        void draw();

        void addParticle(ofPoint pos, ofPoint vel, ofColor color, float radius, float lifetime);
        void addParticles(int n);
        void addParticles(int n, const irMarker& marker);
        void addParticles(int n, const ofPolyline& contour, Contour& flow);

        void createParticleGrid(int width, int height);

        void removeParticles(int n);
        void killParticles();
        void bornParticles();
        void resetTouchedParticles();

        void setAnimation(Animation animation);

        //--------------------------------------------------------------
        bool isActive;          // Particle system active
        //--------------------------------------------------------------
        bool activeStarted;     // Active has started?
        bool isFadingIn;        // Opacity fading in?
        bool isFadingOut;       // Opacity fading out?
        bool startFadeIn;       // Fade in has started?
        bool startFadeOut;      // Fade out has started?
        float elapsedFadeTime;  // Elapsed time of fade
        float fadeTime;         // Transition time of fade
        //--------------------------------------------------------------
        int width;              // Particle system boundaries
        int height;
        //--------------------------------------------------------------
        float opacity;
        float maxOpacity;   
        //--------------------------------------------------------------
        vector<Particle *> particles;
        //--------------------------------------------------------------
        int numParticles;
        int totalParticlesCreated;
        //--------------------------------------------------------------
        ParticleMode particleMode;
        //--------------------------------------------------------------
        Animation animation;
        //--------------------------------------------------------------
        // General properties
        bool immortal;              // Can the particles die?
        float velocity;             // Initial velocity magnitude of newborn particles
        float radius;               // Radius of the particles
        float lifetime;             // Lifetime of particles
        float red, green, blue;     // Color of the particles
        //--------------------------------------------------------------
        // Specific properties
        int nParticles;             // Number of particles born in the beginning
        float bornRate;             // Number of particles born per frame
        //--------------------------------------------------------------
        // Emitter
        float emitterSize;          // Size of the emitter area
        float velocityRnd;          // Magnitude randomness % of the initial velocity
        float velocityMotion;       // Marker motion contribution to the initial velocity
        float lifetimeRnd;          // Randomness of lifetime
        float radiusRnd;            // Randomness of radius
        //--------------------------------------------------------------
        // Grid
        int gridRes;                // Resolution of the grid
        //--------------------------------------------------------------
        // Flocking
        float lowThresh;            // If dist. ratio lower than lowThresh separate
        float highThresh;           // Else if dist. ratio higher than highThresh attract. Else just align
        float separationStrength;   // Separation force
        float attractionStrength;   // Attraction force
        float alignmentStrength;    // Alignment force
        float maxSpeed;             // Max speed particles
        float flockingRadius;       // Radius of flocking
        //--------------------------------------------------------------
        // Graphic output
        bool sizeAge;               // Particles change size with age?
        bool opacityAge;            // Particles change opacity with age?
        bool colorAge;              // Particles change color with age?
        bool flickersAge;           // Particles flicker opacity when about to die?
        bool isEmpty;               // Particles are empty inside, only draw the contour?
        bool drawLine;              // Draw a line instead of a circle for the particle?
        bool drawStroke;            // Draw stroke line around particle?
        float strokeWidth;          // Stroke line width
        bool drawConnections;       // Draw a connecting line between close particles?
        float connectDist;          // Maximum distance to connect particles with line
        float connectWidth;         // Connected line width
        //--------------------------------------------------------------
        // Physics
        float friction;             // Friction to velocity 0~100
        ofPoint gravity;            // Makes particles react to gravity
        float turbulence;           // Turbulence perlin noise
        bool bounce;                // Particles bounce with the window margins?
        bool steer;                 // Particles steer direction with the window margins?
        bool infiniteWalls;         // Infinite walls?
        bool bounceDamping;         // Decrease velocity when particle bounces walls?
        bool repulse;               // Repulse particles between each other?
        float repulseDist;          // Repulse particle-particle distance
        float returnToOriginForce;  // How fast particle returns to its position
        //--------------------------------------------------------------
        // Behavior
        bool interact;              // Can we interact with the particles?
        bool emit;                  // Born new particles in each frame?
        bool flock;                 // Particles have flocking behavior?
        bool flowInteraction;       // Interact with particles using optical flow?
        bool fluidInteraction;      // Interact with the fluid velocities?
        bool repulseInteraction;    // Repulse particles from input?
        bool attractInteraction;    // Attract particles to input?
        bool seekInteraction;       // Make particles seek target (markers)?
        bool gravityInteraction;    // Apply gravity force to particles touched with input?
        bool bounceInteraction;     // Bounce particles with depth contour?
        bool returnToOrigin;        // Make particles return to their born position?
        //--------------------------------------------------------------
        // Input
        bool markersInput;          // Input are the IR markers?
        bool contourInput;          // Input is the depth contour?
        float interactionForce;     // Force of interaction
        float interactionRadius;    // Radius of interaction
        bool emitInMovement;        // Emit particles only in regions that there has been some movement?
        bool emitAllTimeInside;     // Emit particles every frame inside all the defined area?
        bool emitAllTimeContour;    // Emit particles every frame only on the contour of the defined area?
        bool useFlow;               // Use optical flow to get the motion velocity?
        bool useFlowRegion;         // Use optical flow region to get the motion velocity?
        bool useContourArea;        // Use contour area to interact with particles?
        bool useContourVel;         // Use contour velocities to interact with particles?

    protected:
        // Helper functions
        ofPoint randomVector();
        float randomRange(float percentage, float value);
        irMarker* getClosestMarker(const Particle& particle, vector<irMarker>& markers, float interactionRadiusSqrd);
        irMarker* getClosestMarker(const Particle &particle, vector<irMarker>& markers);
        ofPoint getClosestPointInContour(const Particle& particle, const Contour& contour, bool onlyInside = true, unsigned int* contourIdx = NULL);
    
        void fadeIn(float dt);
        void fadeOut(float dt);
    
        void repulseParticles();
        void flockParticles();
};
