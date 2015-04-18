#pragma once
#include "ofMain.h"
#include "ofxFlowTools.h"
#include "irMarker.h"
#include "Contour.h"

class Fluid{
    public:
        Fluid();

        void setup(int flowWidth, int flowHeight, int drawWidth, int drawHeight, bool doFasterInternalFormat = false);
        void update(float dt, vector<irMarker> &markers, Contour &contour);
        void draw();
        
        //--------------------------------------------------------------
        bool isActive;          // Fluid active
        //--------------------------------------------------------------
        int width;              // Fluid boundaries
        int height;
        //--------------------------------------------------------------
        float red, green, blue;
        //--------------------------------------------------------------
        float opacity;
        //--------------------------------------------------------------
        bool markersInput;          // Input are the IR markers?
        bool contourInput;          // Input is the depth contour?
        float markerRadius;         // Radius of interaction of the markers
        //--------------------------------------------------------------
        bool drawVelocity;
        bool drawVelocityScalar;
        //--------------------------------------------------------------
        // fluid parameters
        float speed;
        float cellSize;
        int numJacobiIterations;
        float viscosity;
        float vorticity;
        float dissipation;
        float dissipationVelocityOffset;
        float dissipationDensityOffset;
        float dissipationTemperatureOffset;
        float smokeSigma;
        float smokeWeight;
        float ambientTemperature;
        ofPoint gravity;
        float clampForce;
        float maxVelocity;
        float maxDensity;
        float maxTemperature;
        float densityFromVorticity;
        float densityFromPressure;
        //--------------------------------------------------------------
        // particle flow parameters
        bool isParticlesActive;   // Particle flow active
        float particlesVelocity;
        float particlesCellSize;
        float particlesBirthChance;
        float particlesBirthVelocityChance;
        float particlesLifetime;
        float particlesLifetimeRnd;
        float particlesMass;
        float particlesMassRnd;
        float particlesSize;
        float particlesSizeRnd;

    protected:
        ftFluidSimulation fluid;
        ftParticleFlow particleFlow;
    
        ftDisplayScalar displayScalar;
        ftVelocityField	velocityField;
};
