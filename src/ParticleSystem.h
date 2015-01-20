#pragma once
#include "ofMain.h"
#include "Particle.h"

enum ParticleMode {GRID_PARTICLES, BORN_PARTICLES, CONTOUR_PARTICLES};
enum EmitterType {POINT, SPHERE, GRID, CONTOUR};

class ParticleSystem
{
    public:
        ParticleSystem();

        void setup(ParticleMode particleMode);
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
        float bornRate;
        float currentBornRate;
        float velocity;
        float velocityRnd;
        float velocityMotion;
        float emitterSize;
        float lifetime;
        float lifetimeRnd;
        float radius;
        float radiusRnd;
        ofColor color;

        float friction;
        float gravity;
        float turbulence;

        bool sizeAge;           //particles change size with increasing age
        bool opacityAge;        //particles change opacity with increasing age
        bool colorAge;          //particles change color with increasing age
        bool bounce;            //particles bounce with the window margins?

        bool killingParticles;

        EmitterType emitterType;
        ParticleMode particleMode;
};
