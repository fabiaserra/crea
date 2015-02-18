#pragma once
#include "ofMain.h"
#include "Particle.h"
#include "irMarker.h"

enum ParticleMode {GRID_PARTICLES, MARKER_PARTICLES, CONTOUR_PARTICLES};

class ParticleSystem
{
	public:
		ParticleSystem();

		void setup(ParticleMode mode);
		void update(float dt, vector<irMarker>& markers);
		void draw();

        void addParticle(ofPoint pos, ofPoint vel, ofColor color, float radius, float lifetime);
		void addParticles(int n);
		void addParticles(int n, const irMarker &marker);
//		void addParticles(int n, const ofPolyline &contour);

        void createParticleGrid(int width, int height, int res);

		void removeParticles(int n);

		void killParticles();
		void repulseParticles();

		//--------------------------------------------------------------
		bool isActive;          // Particle system active
		//--------------------------------------------------------------
        int width;              // Particle system boundaries
        int height;
		//--------------------------------------------------------------
		vector<Particle> particles;
		//--------------------------------------------------------------
		int numParticles;
		int totalParticlesCreated;
		//--------------------------------------------------------------
		ofColor color;
		ParticleMode particleMode;
		//--------------------------------------------------------------
		float bornRate;         // Number of particles born per frame
		float velocity;         // Initial velocity magnitude of newborn particles
		float velocityRnd;      // Magnitude randomness % of the initial velocity
		float velocityMotion;   // Marker motion contribution to the initial velocity
		float emitterSize;      // Size of the emitter area
		float lifetime;         // Lifetime of particles
		float lifetimeRnd;      // Randomness of lifetime
		float radius;           // Radius of the particles
		float radiusRnd;        // Randomness of radius
		//--------------------------------------------------------------
		bool immortal;          // Can the particles die?
		bool sizeAge;           // Particles change size with age?
		bool opacityAge;        // Particles change opacity with age?
		bool flickersAge;       // Particles flicker opacity when about to die?
		bool colorAge;          // Particles change color with age?
		bool bounce;            // Particles bounce with the window margins?
		bool isEmpty;           // Particles are empty inside, only draw the contour?
		//--------------------------------------------------------------
		float friction;        	// Multiply this value by the velocity every frame
        float gravity;          // Makes particles fall down in a natural way

	protected:
		// helper functions
		ofPoint randomVector();
		float randomRange(float percentage, float value);
};
