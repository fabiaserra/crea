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
        void draw();

        void createParticleGrid(int width, int height, int res);
        void addParticles(int n);
        void addParticles(int n, const ofPoint &markerPos, const ofPoint &markerVel, const ofColor &markerColor);
        void addParticle(int x, int y, float initialRadius);
        void removeParticles(int n);

        void killParticles();
        void repulseParticles();

        ofPoint randomVector();
        float randomRange(float percentage, float value);

        vector<Particle> particles;

        int numParticles;
        int totalParticlesCreated;

        ofColor color;
        ParticleMode particleMode;
        EmitterType emitterType;

        //Particles parameters
        float bornRate;         //Number of particles born per frame

        float velocity;         //Initial velocity magnitude of newborn particles
        float velocityRnd;      //Magnitude randomness % of the initial velocity
        float velocityMotion;   //Marker motion contribution to the initial velocity

        float emitterSize;      //Size of the emitter area

        float lifetime;         //Lifetime of particles
        float lifetimeRnd;      //Randomness of lifetime
        float radius;           //Radius of the particles
        float radiusRnd;        //Randomness of radius

        bool immortal;          //can the particles die?
        bool sizeAge;           //particles change size with age?
        bool opacityAge;        //particles change opacity with age?
        bool colorAge;          //particles change color with age?
        bool bounce;            //particles bounce with the window margins?

        float friction;         //Multiply this value by the velocity every frame
        float gravity;          //Makes particles fall down in a natural way
};
