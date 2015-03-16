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
        void addNoise(float angle, float turbulence, float dt);
        void addRepulsionForce(float x, float y, float radiusSqrd, float scale);
        void addAttractionForce(float x, float y, float radiusSqrd, float scale);
        void addRepulsionForce(Particle &p, float radiusSqrd, float scale);
        void addAttractionForce(Particle &p, float radiusSqrd, float scale);
        void addRepulsionForce(Particle &p, float scale);
        void xenoToOrigin(float spd);

        void addFlockingForces(Particle &p);
        void seek(ofPoint target, float radiusSqrd);
        void pullToCenter();
        void limitVelocity();

        void bounceParticle();
        void steerParticle();
        void marginsWrap();

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

        bool limitSpeed;        // Limit the speed of the particle?
        bool bounceDamping;     // Decrease velocity when particle bounces?

        bool bounces;           // Particle bounces with the window margins?
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
// --------------------------------------------------------------
        int width;              // Particle boundaries
        int height;
};

