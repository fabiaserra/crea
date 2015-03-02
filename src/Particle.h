#pragma once
#include "ofMain.h"
#include "irMarker.h"

class flockingForce {
public:
    ofPoint     sum;
    int			count;
    float		distSqrd;
    float		strength;
};

class Particle
{
    public:
        Particle();

        void setup(float id, ofPoint pos, ofPoint vel, ofColor color, float initialRadius, float lifetime);
        void update(float dt);
        void draw();

        void addForce(ofPoint force);
        void addFlockingForces();
        void addNoise(float angle, float turbulence, float dt);
        void addRepulsionForce(float x, float y, float radiusSqrd, float scale);
        void addAttractionForce(float x, float y, float radiusSqrd, float scale);
        void addRepulsionForce(Particle &p, float radiusSqrd, float scale);
        void addAttractionForce(Particle &p, float radiusSqrd, float scale);
        void addRepulsionForce(Particle &p, float scale);
        void xenoToOrigin(float spd);

        void addForFlocking(Particle &p);
        void seek(ofPoint target, float maxSpeed);
        void limitVelocity();

        void kill();
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
        float opacity;          // Radius of the particle
        float noise;            // Perlin noise to add some randomness
        float originalHue;
// --------------------------------------------------------------
        bool  immortal;         // Can the particle die?
        bool  isAlive;          // Is the particle alive?
        bool  isTouched;        // Particle has been activated through some event
        bool  bounces;          // Particle bounces with the window margins?
        bool  sizeAge;          // Particle changes size with age?
        bool  opacityAge;       // Particle changes opacity with age?
        bool  flickersAge;      // Particle flickers opacity when about to die?
        bool  colorAge;         // Particle changes color with age?
        bool  isEmpty;          // Draw only contour of the particle
        bool  drawLine;         // Draw particle as a line from prevPos to pos
// --------------------------------------------------------------
        int width;              // Particle boundaries
        int height;    
// --------------------------------------------------------------
        flockingForce separation;
        flockingForce alignment;
        flockingForce cohesion;
};

