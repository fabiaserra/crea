#pragma once
#include "ofMain.h"
#include "Particle.h"

enum ParticleMode {GRID_PARTICLES, BORN_PARTICLES, CONTOUR_PARTICLES};
enum EmitterType {POINT, SPHERE, BOX, GRID, CONTOUR};

class ParticleSystem
{
    public:
        ParticleSystem();

        void setup(ParticleMode particleMode);
        void update(float dt, const ofPoint &pos, const ofPoint &vel);
        void draw();

        void createParticleGrid(int width, int height, int res);
        void addParticles(int n);
        void addParticles(int n, const ofPoint &pos_, const ofPoint &vel_);
        void addParticle(int x, int y, float initialRadius);
        void removeParticles(int n);

        void killParticles();
        void bornParticles();
        void repulseParticles();

        void setBornRate(float bornRate);
        void setParticleMode(ParticleMode particleMode);

        ofPoint randomVector();


        vector<Particle> particles;

        int numParticles;
        int totalParticlesCreated;

        float bornRate;
        float emitterSize;
        EmitterType emitterType;
        float lifetime;
        float lifetimeRnd;

        bool killingParticles;

        ofColor color;

        ParticleMode particleMode;
};
