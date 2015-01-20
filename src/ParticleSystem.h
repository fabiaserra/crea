#pragma once
#include "ofMain.h"
#include "Particle.h"
#include "Marker.h"

enum ParticleMode {GRID_PARTICLES, BORN_PARTICLES, CONTOUR_PARTICLES};
enum EmitterType {POINT, SPHERE, GRID, CONTOUR};

class ParticleSystem
{
    public:
        ParticleSystem();

        void setup(bool immortal, ofColor color, float gravity, bool sizeAge, bool opacityAge, bool colorAge, bool bounce);
        void setup(float bornRate, float velocity, float velocityRnd, float velocityMotion, float emitterSize, bool immortal, float lifetime, float lifetimeRnd,
                   ofColor color, float radius, float radiusRnd, float friction, float gravity, bool sizeAge, bool opacityAge, bool colorAge, bool bounce);

        void update(float dt, vector<Marker>& markers);
        void update(float dt, const ofPoint &pos, const ofPoint &markerVel);
        void draw();

        void createParticleGrid(int width, int height, int res);
        void addParticles(int n);
        void addParticles(int n, const ofPoint &pos, const ofPoint &markerVel);
        void addParticle(int x, int y, float initialRadius);
        void removeParticles(int n);

        void killParticles();
        void bornParticles();
        void repulseParticles();

        ofPoint randomVector();
        float randomRange(float percentage, float value);

        vector<Particle> particles;

        int numParticles;
        int totalParticlesCreated;

        //Particles parameters
        ofColor color;
        float currentBornRate;

        float bornRate;
        float velocity;
        float velocityRnd;
        float velocityMotion;
        float emitterSize;
        float lifetime;
        float lifetimeRnd;
        float radius;
        float radiusRnd;

        float friction;
        float gravity;
//        float turbulence;

        bool immortal;          //can the particles die?
        bool sizeAge;           //particles change size with age?
        bool opacityAge;        //particles change opacity with age?
        bool colorAge;          //particles change color with age?
        bool bounce;            //particles bounce with the window margins?

        bool killingParticles;

        EmitterType emitterType;
        ParticleMode particleMode;
};
