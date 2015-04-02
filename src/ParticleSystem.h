#pragma once
#include "ofMain.h"
#include "Particle.h"
#include "irMarker.h"
#include "Contour.h"

enum ParticleMode {EMITTER, BOIDS, GRID, RANDOM, ANIMATIONS};
enum InputSource {MARKERS, CONTOUR};
enum Animation {SNOW, RAIN, WIND, EXPLOSION};

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
		void addParticles(int n, const ofPolyline &contour, Contour &flow);

        void createParticleGrid(int width, int height);

		void removeParticles(int n);

		void killParticles();
        void bornParticles();
		void repulseParticles();
		void flockParticles();

        void setAnimation(Animation animation);

		//--------------------------------------------------------------
		bool isActive;          // Particle system active
		//--------------------------------------------------------------
        int width;              // Particle system boundaries
        int height;
		//--------------------------------------------------------------
        float opacity;
        //--------------------------------------------------------------
		vector<Particle *> particles;
		//--------------------------------------------------------------
		int numParticles;
		int totalParticlesCreated;
		//--------------------------------------------------------------
		ParticleMode particleMode;
		//--------------------------------------------------------------
		Animation animation;
		//--------------------------------------------------------------
		// General properties
        bool immortal;              // Can the particles die?
		float velocity;             // Initial velocity magnitude of newborn particles
        float radius;               // Radius of the particles
        float lifetime;             // Lifetime of particles
        ofColor color;              // Color of the particles
        //--------------------------------------------------------------
        // Specific properties
        float nParticles;           // Number of particles born in the beginning
        float bornRate;             // Number of particles born per frame
        //--------------------------------------------------------------
        // Emitter
		float emitterSize;          // Size of the emitter area
		float velocityRnd;          // Magnitude randomness % of the initial velocity
		float velocityMotion;       // Marker motion contribution to the initial velocity
		float lifetimeRnd;          // Randomness of lifetime
		float radiusRnd;            // Randomness of radius
        //--------------------------------------------------------------
		// Grid
		int gridRes;                // Resolution of the grid
        //--------------------------------------------------------------
		// Flocking
        float lowThresh;            // If dist. ratio lower than lowThresh separate
        float highThresh;           // Else if dist. ratio higher than highThresh attract. Else just align
        float separationStrength;   // Separation force
        float attractionStrength;   // Attraction force
        float alignmentStrength;    // Alignment force
        float maxSpeed;             // Max speed particles
        float flockingRadius;       // Radius of flocking
        //--------------------------------------------------------------
		// Graphic output
		bool sizeAge;               // Particles change size with age?
		bool opacityAge;            // Particles change opacity with age?
		bool colorAge;              // Particles change color with age?
		bool flickersAge;           // Particles flicker opacity when about to die?
		bool isEmpty;               // Particles are empty inside, only draw the contour?
		bool drawLine;              // Draw a line instead of a circle for the particle?
        bool drawConnections;       // Draw a connecting line between close particles?
		//--------------------------------------------------------------
		// Physics
		float friction;        	    // Friction to velocity 0~100
        float gravity;              // Makes particles fall down in a natural way
        float turbulence;           // Turbulence perlin noise
		bool repulse;               // Repulse particles between each other?;
		bool bounce;                // Particles bounce with the window margins?
		bool steer;                 // Particles steer direction with the window margins?
		bool infiniteWalls;         // Infinite walls?
        bool bounceDamping;         // Decrease velocity when particle bounces walls?
        //--------------------------------------------------------------
        // Behavior
        bool interact;              // Can we interact with the particles?
        bool emit;                  // Born new particles in each frame?
        bool flock;                 // Particles have flocking behavior?
        bool repulseInteraction;    // Repulse particles from input?
        bool attractInteraction;    // Attract particles to input?
        bool gravityInteraction;    // Apply gravity force to particles touched with input?
        bool returnToOrigin;        // Make particles return to their born position?
        //--------------------------------------------------------------
        // Input
        bool markersInput;          // Input are the IR markers?
        bool contourInput;          // Input is the depth contour?
        float markerRadius;         // Radius of interaction of the markers
        bool emitInMovement;        // Emit particles only in regions that there has been some movement?
        bool emitAllTimeInside;     // Emit particles every frame inside all the defined area?
        bool emitAllTimeContour;    // Emit particles every frame only on the contour of the defined area?
        bool useFlow;               // Use optical flow to get the motion velocity?
        bool useFlowRegion;         // Use optical flow region to get the motion velocity?
        bool useContourArea;        // Use contour area to interact with particles?
        bool useContourVel;         // Use contour velocities to interact with particles?


	protected:
		// Helper functions
		ofPoint randomVector();
		float randomRange(float percentage, float value);
		ofPoint getClosestMarker(const Particle &particle, const vector<irMarker> &markers, float markerRadius);
//		irMarker getClosestMarker(const Particle &particle, const vector<irMarker> &markers, float markerRadius);
		ofPoint getClosestPointInContour(const Particle &particle, const Contour &contour);
};
