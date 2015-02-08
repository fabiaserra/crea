#pragma once
#include "ofMain.h"
#include "irMarker.h"

class Particle
{
    public:
        Particle();

        void setup(float id, ofPoint pos, ofPoint vel, ofColor color, float initialRadius, bool immortal, float lifetime, float friction);
        void setup(float id, ofPolyline contour, ofPoint vel, ofColor color, float initialRadius, bool immortal, float lifetime, float friction);
        void update(float dt);
        void update(float dt, vector<irMarker>& markers);
        void draw();
        void kill();
//--------------------------------------------------------------
        ofPoint pos;            // Position
        ofPoint prevPos;        // Previous position
        ofPoint vel;            // Velocity
        ofPoint acc;            // Acceleration
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
        float opacity;          // Radius of the particle
        float noise;            // Perlin noise to add some randomness
        float originalHue;
// --------------------------------------------------------------
        bool  immortal;          // Can the particle die?
        bool  isAlive;           // Is the particle alive?
        bool  bounces;           // Particle bounces with the window margins?
        bool  sizeAge;           // Particle changes size with age?
        bool  opacityAge;        // Particle changes opacity with age?
        bool  flickersAge;       // Particle flickers opacity when about to die?
        bool  colorAge;          // Particle changes color with age?
        bool  drawContour;       // Draw only contour of the particle
        bool  drawShapes;        // Draw other shapes as particles
// --------------------------------------------------------------
        float markerDist;
        ofPoint dir;
};

