#pragma once
#include "ofMain.h"
#include "Particle.h"
#include "irMarker.h"

enum ParticleMode {GRID_PARTICLES, BORN_PARTICLES, CONTOUR_PARTICLES};
enum EmitterType {POINT, SPHERE, GRID, CONTOUR};

class ParticleSystem
{
	public:
		ParticleSystem();

		void setup(bool immortal, ofColor color, float radius, bool sizeAge, bool opacityAge, bool flickersAge,
                   bool colorAge, bool isEmpty, bool bounce);
		void setup(float bornRate, float velocity, float velocityRnd, float velocityMotion, float emitterSize,
				   bool immortal, float lifetime, float lifetimeRnd, ofColor color, float radius, float radiusRnd,
				   float friction, bool sizeAge, bool opacityAge,  bool flickersAge, bool colorAge, bool isEmpty,
                   bool bounce);

		void update(float dt, vector<irMarker>& markers);
		void draw();

		void createParticleGrid(int width, int height, float radius, int res);
		void addParticles(int n, const ofPoint &markerPos, const ofPoint &markerVel, const ofColor &markerColor);
		void addParticle(int x, int y, float initialRadius);
        void addParticles(int n);
		void removeParticles(int n);

		void killParticles();
		void repulseParticles();

		//--------------------------------------------------------------
		bool isActive;
		//--------------------------------------------------------------
		vector<Particle> particles;
		//--------------------------------------------------------------
		int numParticles;
		int totalParticlesCreated;
		//--------------------------------------------------------------
		ofColor color;
		ParticleMode particleMode;
		EmitterType emitterType;
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
