#pragma once
#include "ofMain.h"
#include "Particle.h"
#include "irMarker.h"
#include "Contour.h"

enum ParticleMode {EMITTER, BOIDS, GRID, RANDOM};
enum InputSource {MARKERS, CONTOUR};

class ParticleSystem
{
	public:
		ParticleSystem();
		~ParticleSystem();

		void setup(ParticleMode particleMode, InputSource inputSource, int width, int height);
		void update(float dt, vector<irMarker> &markers, Contour &contour);
		void draw();

        void addParticle(ofPoint pos, ofPoint vel, ofColor color, float radius, float lifetime);
		void addParticles(int n);
		void addParticles(int n, const irMarker &marker);
		void addParticles(int n, const ofPolyline &contour);

        void createParticleGrid(int width, int height);

		void removeParticles(int n);

		void killParticles();
        void bornParticles();
		void repulseParticles();
		void flockParticles();

		//--------------------------------------------------------------
		bool isActive;          // Particle system active
		//--------------------------------------------------------------
        int width;              // Particle system boundaries
        int height;
		//--------------------------------------------------------------
		vector<Particle *> particles;
		//--------------------------------------------------------------
		int numParticles;
		int totalParticlesCreated;
		//--------------------------------------------------------------
		ofColor color;
		ParticleMode particleMode;
		InputSource inputSource;
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
		int gridRes;            // Resolution of the grid
		int nParticles;         // Number of particles
		//--------------------------------------------------------------
		bool immortal;          // Can the particles die?
		bool sizeAge;           // Particles change size with age?
		bool opacityAge;        // Particles change opacity with age?
		bool flickersAge;       // Particles flicker opacity when about to die?
		bool colorAge;          // Particles change color with age?
		bool bounce;            // Particles bounce with the window margins?
		bool isEmpty;           // Particles are empty inside, only draw the contour?
		bool drawLine;          // Particles are circles or we draw a line from prevpos to pos?
		//--------------------------------------------------------------
		float friction;        	// Multiply this value by the velocity every frame
        float gravity;          // Makes particles fall down in a natural way

	protected:
		// helper functions
		ofPoint randomVector();
		float randomRange(float percentage, float value);
		ofPoint getClosestMarker(const Particle &particle, const vector<irMarker> &markers, float markerRadius);
};
