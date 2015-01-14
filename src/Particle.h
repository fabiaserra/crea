#pragma once
#include "ofMain.h"

class Particle
{
    public:
        Particle();

        void setup(float id, ofPoint pos, ofPoint vel, ofColor color, float initialRadius, float lifetime, float friction);
        void setup(float id, ofPolyline contour, ofPoint vel, ofColor color, float initialRadius, float lifetime, float friction);
        void update(float dt, const ofPoint &markerPos);
        void draw();

        void kill();

        ofPoint pos;            //position
        ofPoint prevPos;        //previous position
        ofPoint vel;		    //velocity
        ofPoint acc;            //acceleration
        ofColor color;          //color

        ofPolyline contour;     //particle creation boundaries

        float id;               //particle ID
        float age;              //time of living
        float mass;             //mass of the particle
        float lifetime;         //allowed lifetime
        float friction;         //decay of the velocity
        float initialRadius;    //radius of the particle when borns
        float radius;           //radius of the particle
        float noise;            //perlin noise to add some randomness
        float originalHue;

        bool isAlive;           //is the particle alive?
        bool bounces;           //particle bounces with the window margins?

        ofPoint dir;
};

