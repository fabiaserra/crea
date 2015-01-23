#pragma once
#include "ofMain.h"
#include "Marker.h"

class Particle
{
    public:
        Particle();

        void setup(float id, ofPoint pos, ofPoint vel, ofColor color, float initialRadius, bool immortal, float lifetime, float friction);
        void setup(float id, ofPolyline contour, ofPoint vel, ofColor color, float initialRadius, bool immortal, float lifetime, float friction);
        void update(float dt);
        void update(float dt, vector<Marker>& markers);
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
        float opacity;           //radius of the particle
        float noise;            //perlin noise to add some randomness
        float originalHue;

        bool  immortal;          //can the particle die?
        bool  isAlive;           //is the particle alive?
        bool  bounces;           //particle bounces with the window margins?
        bool  sizeAge;           //particle changes size with age?
        bool  opacityAge;        //particle changes opacity with age?
        bool  flickersAge;       //particle flickers opacity when about to die?
        bool  colorAge;          //particle changes color with age?

        float markerDist;

        ofPoint dir;

};

